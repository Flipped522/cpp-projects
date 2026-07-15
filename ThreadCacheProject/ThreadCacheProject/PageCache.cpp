#define _CRT_SECURE_NO_WARNINGS 1 

#include "PageCache.h"

PageCache PageCache::_sInst;

Span* PageCache::NewSpan(size_t k)
{

	assert(k > 0 && k < NPAGES);


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
				Span* kSpan = new Span;
				// 在nSpan的头部切一个k页下来
				// k页的span返回，nSpan挂到对应的位置
				kSpan->_page_Id = nSpan->_page_Id;
				kSpan->_n = k;

				nSpan->_page_Id += k;
				nSpan->_n -= k;

				_spanLists[nSpan->_n].PushFront(nSpan);

				return kSpan;
			}
		}
	}
	// 到这里说明没有大页span
	// 找堆去要一个128页的span
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_page_Id = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}
