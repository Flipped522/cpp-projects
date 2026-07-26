#define _CRT_SECURE_NO_WARNINGS 1 

#include "Common.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	Span* MapObjectToSpan(void* obj);

	// 获取一个K页的span
	Span* NewSpan(size_t k);

	void ReleaseSpanToPageCache(Span* span);

	std::mutex _pageMtx;
private:
	PageCache()
	{

	}

	PageCache(const PageCache&) = delete;

	SpanList _spanLists[NPAGES];
	std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	
	static PageCache _sInst;
};