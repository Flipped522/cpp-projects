#define _CRT_SECURE_NO_WARNINGS 1 
#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (batchNum == _freeLists[index].MaxSize())
	{
		_freeLists[index].MaxSize() += 1;
	}

	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum >= 1);

	if (actualNum == 1)
	{
		assert(start == end);
		return start;
	}

	else
	{
		_freeLists[index].PushRange(NextObj(start), end);
		return start;
	}

}

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

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}
