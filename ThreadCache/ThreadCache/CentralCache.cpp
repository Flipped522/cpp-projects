#define _CRT_SECURE_NO_WARNINGS 1 
#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 查看当前spanlist是否还有未分配的span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr)
		{
			return it;
		}
		else
		{
			it = it->_next;
		}
	}
	// 将central cache的桶锁解掉，可以保证其它线程释放内存对象时，不会被阻塞
	list._mtx.unlock();

	// 没有空闲span，向span cache获取
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// 对获取的span进行切分

	// 计算span大块内存的起始地址和大块内存的大小（字节数）
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// 将大块内存切成自由链表链接起来
	// 1. 先切一块
	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;
	int i = 1;
	while (start < end)
	{
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}

	// 切好span以后，将span挂到桶里，加锁
	list._mtx.lock();
	list.PushFront(span);

	return span;
	
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// 从span中获取batchNum个对象
	// 如果不够batchNum个，有多少取多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		++i;
		++actualNum;
	}

	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_usecount == actualNum;

	_spanLists[index]._mtx.unlock();

	return actualNum;
}