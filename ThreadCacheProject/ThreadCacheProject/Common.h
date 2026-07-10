#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <cassert>
using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;

void*& NextObj(void* obj)
{
	return *(void**)obj;
}

// 管理切分好的小对象的自由链表
class FreeList
{
public:
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
	}
private:
	void* _freeList;
};