#define _CRT_SECURE_NO_WARNINGS 1 
#include "ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);
	size_t index = SizeClass::Index(size);

	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else
	{
		return FetchFromCentralCache(index, alignSize);
	}

	return nullptr;
}

void ThreadCache::Dellocate(void* ptr, size_t size)
{
	assert(size <= MAX_BYTES);
	assert(nullptr != ptr);

	// 找出对应的映射的自由链表的桶并插入
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return nullptr;
}
