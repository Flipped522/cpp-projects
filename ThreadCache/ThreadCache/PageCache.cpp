#define _CRT_SECURE_NO_WARNINGS 1 

#include "PageCache.h"

PageCache PageCache::_sInst;

// 获取一个k页的span
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0 && k < NPAGES);

	// 检查第k个桶里有没有span
	if (!_spanLists[k].Empty())
	{
		return _spanLists->PopFront();
	}

	// 检查一下后面的桶有没有span，有的话进行切分
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nSpan = _spanLists[i].PopFront();
			Span* kSpan = new Span;

			// 在nSpan的头部切一个k页下来
			// k页Span返回
			// nSpan再挂到对应的位置
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;
			nSpan->_n -= k;

			_spanLists[nSpan->_n].PushFront(nSpan);

			return kSpan;
		}

	}

	// 向堆申请128页的Span
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}