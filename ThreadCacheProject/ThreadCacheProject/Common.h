#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <thread>
#include <mutex>
#include <cassert>
using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;
static const size_t NFREELISTS = 208;
static const size_t NPAGES = 128;
static const size_t PAGE_SHIFT = 13;

#ifdef _WIN64
	typedef unsigned long long PAGE_ID;
#elif _WIN32
	typedef size_t PAGE_ID;
#else
	// Linux
#endif

static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

// 管理切分好的小对象的自由链表
class FreeList
{
public:
	void PushRange(void* start, void* end)
	{
		NextObj(end) = nullptr;
		_freeList = start;
	}

	void Push(void* obj)
	{
		// 头插
		NextObj(obj) = _freeList;
		_freeList = obj;
	}

	void* Pop()
	{
		assert(_freeList);
		// 头删
		void* obj = _freeList;
		_freeList = NextObj(obj);
		return obj;
	}

	bool Empty()
	{
		return nullptr == _freeList;
	}

	size_t& MaxSize()
	{
		return _maxSize;
	}
private:
	void* _freeList = nullptr;
	size_t _maxSize = 1;
};

// 计算对象大小的对齐映射规则
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费 
	// [1,128]				8byte对齐		freelist[0,16) 
	// [128+1,1024]			16byte对齐		freelist[16,72) 
	// [1024+1,8*1024]		128byte对齐		freelist[72,128) 
	// [8*1024+1,64*1024]	1024byte对⻬	freelist[128,184) 
	// [64*1024+1,256*1024] 8*1024byte对⻬	freelist[184,208) 
	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return  ((bytes + alignNum - 1) & ~(alignNum - 1));
	}

	static size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			assert(false);
			return -1;
		}
	}

	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);
		static int group_array[4] = { 16,56,56,56 };
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024)
		{
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024)
		{
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024)
		{
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (bytes <= 256 * 1024)
		{
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else
		{
			assert(false);
		}
		return -1;
	}
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;

		return num;
	}

	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;
		if (0 == npage)
			npage = 1;

		return npage;
	}
};

// 管理多个连续页大块内存跨度结构
struct Span
{
	PAGE_ID _page_Id = 0; // 大块内存起始页的页号
	size_t _n = 0;		  // 页的数量

	Span* _next = nullptr;	  // 双向链表结构
	Span* _prev = nullptr;	  

	size_t _usecount = 0; // 切好的小块内存，被分配给thread cache的计数
	void* _freeList = nullptr;  // 切好的小块内存的自由链表
};

// 带头双向循环链表
class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);
	}

	void Insert(Span* pos, Span* newSpan)
	{
		assert(nullptr != pos);
		assert(nullptr != newSpan);

		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_prev = prev;
		newSpan->_next = pos;
		pos->_prev = newSpan;
	}

	void Erase(Span* pos)
	{
		assert(nullptr != pos);
		assert(_head != pos);

		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}
public:
	std::mutex _mtx; // 桶锁
private:
	Span* _head;
};