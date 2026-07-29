#define _CRT_SECURE_NO_WARNINGS 1 

#include "PageCache.h"

PageCache PageCache::_sInst;

Span* PageCache::MapObjectToSpan(void* obj)
{
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);
	std::unique_lock<std::mutex> lock(_pageMtx);

	auto ret = _idSpanMap.find(id);
	if (_idSpanMap.end() != ret)
	{
		return ret->second;
	}
	else
	{
		assert(false);
		return nullptr;
	}
}

Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0);
	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		//Span* span = new Span;
		Span* span = _spanPool.New();
		span->_page_Id = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		_idSpanMap[span->_page_Id] = span;
	
		return span;
	}

	if(!_spanLists[k].Empty())
	{
		return _spanLists->PopFront();
	}
	else
	{
		// 检查后面的桶里面有没有span，如果有，将大的span进行切分。
		for (size_t i = k +1; i < NPAGES; ++i)
		{
			if (!_spanLists[i].Empty())
			{
				Span* nSpan = _spanLists[i].PopFront();
				Span* kSpan = _spanPool.New();
				// 在nSpan的头部切一个k页下来
				// k页的span返回，nSpan挂到对应的位置
				kSpan->_page_Id = nSpan->_page_Id;
				kSpan->_n = k;

				nSpan->_page_Id += k;
				nSpan->_n -= k;

				_spanLists[nSpan->_n].PushFront(nSpan);
				// 存储nSpan首尾页号和nSpan的映射，方便page cache回收内存时进行合并查找
				_idSpanMap[nSpan->_page_Id] = nSpan;
				_idSpanMap[nSpan->_page_Id + nSpan->_n - 1] = nSpan;

				// 建立id和Span的映射，方便central cache回收小块内存时，查找对应的span
				for (PAGE_ID i = 0; i < kSpan->_n; ++i)
				{
					_idSpanMap[kSpan->_page_Id + i] = kSpan;
				}
				return kSpan;
			}
		}
	}
	// 到这里说明没有大页span
	// 找堆去要一个128页的span
	Span* bigSpan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_page_Id = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
	// 大于128页 page直接向堆申请
	if (span->_n > NPAGES - 1)
	{
		void* ptr = (void*)(span->_page_Id << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanPool.Delete(span);
		return;
	}

	// 对span前后的页，尝试进行合并，缓解内存碎片的问题
	while (1)
	{
		PAGE_ID prevId = span->_page_Id - 1;
		auto ret = _idSpanMap.find(prevId);
		if (_idSpanMap.end() == ret)
		{
			break;
		}

		Span* prevSpan = ret->second;
		if (true == prevSpan->_isUse)
		{
			break;
		}

		// 合并出超过128页的span
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}
		span->_page_Id = prevSpan->_page_Id;
		span->_n += prevSpan->_n;

		_spanLists[prevSpan->_n].Erase(prevSpan);

		//delete prevSpan;
		_spanPool.Delete(prevSpan);
	}

	// 向后合并
	while (1)
	{
		PAGE_ID nextId = span->_page_Id + span->_n;
		auto ret = _idSpanMap.find(nextId);
		if (_idSpanMap.end() == ret)
		{
			break;
		}

		Span* nextSpan = ret->second;
		if (true == nextSpan->_isUse)
		{
			break;
		}
		if (nextSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}
		span->_n += nextSpan->_n;

		_spanLists[nextSpan->_n].Erase(nextSpan);
		//delete nextSpan;
		_spanPool.Delete(nextSpan);
	}
	_spanLists[span->_n].PushFront(span);
	_idSpanMap[span->_page_Id] = span;
	_idSpanMap[span->_page_Id + span->_n - 1] = span;
	span->_isUse = false;
}
