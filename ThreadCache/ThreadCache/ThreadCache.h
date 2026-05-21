#define _CRT_SECURE_NO_WARNINGS 1

#include "Common.h"

#pragma once

class ThreadCache
{
public:
	// 申请和释放内存对象
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	// 从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	FreeList _freeLists[NFREELIST];

};

// TLS thread local storge
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;