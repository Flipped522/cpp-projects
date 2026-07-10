#define _CRT_SECURE_NO_WARNINGS 1 
#include "ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	return nullptr;
}

void ThreadCache::Dellocate(void* ptr, size_t size)
{
}
