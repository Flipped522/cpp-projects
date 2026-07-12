#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once

#include "Common.h"
#include "ThreadCache.h"

static void* ConcurrentAlloc(size_t size)
{
	// 通过TLS 每个线程无锁的获取自己的专属的ThreadCache
	if (nullptr == pTLSThreadCache)
	{
		pTLSThreadCache = new ThreadCache;
	}
	cout << std::this_thread::get_id() << ":" << pTLSThreadCache << endl;

	return pTLSThreadCache->Allocate(size);
}

static void* ConcurrentFree(void* ptr, size_t size)
{
	assert(pTLSThreadCache);

	pTLSThreadCache->Dellocate(ptr,size);
}