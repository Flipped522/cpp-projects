#define _CRT_SECURE_NO_WARNINGS 1 
#include "CentralCache.h"

CentralCache CentralCache::_sInst;

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
    size_t index = SizeClass::Index(size);
    _spanLists[index]._mtx.lock();

    Span* span = GetOneSpan(_spanLists[index], size);
    assert(nullptr != span);
    assert(nullptr != span->_freeList);
    
    // 从Span中获取batchNum个对象
    // 如果不够batchNum个对象，有多少拿多少
    start = span->_freeList;
    end = start;
    size_t i = 0;
    size_t actualNum = 1;
    while(i < batchNum - 1 && nullptr != NextObj(end))
    {
        end = NextObj(end);
        ++i;
        ++actualNum;
    }
    span->_freeList = NextObj(end);
    NextObj(end) = nullptr;

    _spanLists[index]._mtx.unlock();

    return actualNum;
}

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
    // ...
    return nullptr;
}
