#define _CRT_SECURE_NO_WARNINGS 1 
#include "CentralCache.h"
#include "PageCache.h"

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
    span->_usecount += actualNum;

    _spanLists[index]._mtx.unlock();

    return actualNum;
}

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
    // 查看当前的spanlist中是否还有未分配对象的span
    Span* it = list.Begin();
    while (it != list.End())
    {
        if (nullptr != it->_freeList)
        {
            return it;
        }
        else 
        {
            it = it->_next;
        }
    }

    // 先把CentralCache 对应的桶锁解掉，其他线程释放内存对象回来，不会阻塞
    list._mtx.unlock();

    // 没有空闲的span，只能找下一层pageCache
    PageCache::GetInstance()->_pageMtx.lock();
    Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
    PageCache::GetInstance()->_pageMtx.unlock();

    // 对获取的span进行切分，其它线程拿不到这个Span，不需要加锁
    // 
    // 计算span的大块内存的起始地址和大块内存大小的字节数
    char* start = (char*)(span->_page_Id << PAGE_SHIFT);
    size_t bytes = span->_n << PAGE_SHIFT;
    void* end = start + bytes;
    // 把大块内存切成自由链表链接起来，尾插
    // 切一块下来做头
    span->_freeList = start;
    start += size;
    void* tail = span->_freeList;
    while (start < end)
    {
        NextObj(tail) = start;
        tail = start;
        start += size;
    }

    // 切好后需要把span挂到桶里，需要加锁
    list._mtx.lock();
    list.PushFront(span);
    
    return span;
}

void CentralCache::ReleaseListToSpans(void* start, size_t size)
{
    size_t index = SizeClass::Index(size);
    _spanLists[index]._mtx.lock();

    while (nullptr != start)
    {
        void* next = NextObj(start);

        Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
        NextObj(start) = span->_freeList;
        span->_freeList = start;
        --span->_usecount;
        // 说明span切分出去的小块内存都回来了， 该span可以还给page cache，pagecache可以再去做前后页的合并
        if (0 == span->_usecount)
        {
            _spanLists[index].Erase(span);
            span->_freeList = nullptr;
            span->_next = nullptr;
            span->_prev = nullptr;

            // 释放span给page cache时，使用page cache的锁即可，暂时可把桶锁解掉
            _spanLists[index]._mtx.unlock();

            PageCache::GetInstance()->_pageMtx.lock();
            PageCache::GetInstance()->ReleaseSpanToPageCache(span);
            PageCache::GetInstance()->_pageMtx.unlock();

            _spanLists[index]._mtx.lock();
        }
        start = next;
    }

    _spanLists[index]._mtx.unlock();
}
