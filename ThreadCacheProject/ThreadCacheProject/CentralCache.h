#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once

#include "Common.h"

// 单例模式
class CentralCache
{
public:
	// 从中心缓存获取一定数量的对象给thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	// 获取一个非空的Span
	Span* GetOneSpan(SpanList& list, size_t size);
private:
	CentralCache()
	{

	}

	CentralCache(const CentralCache&) = delete;

	SpanList _spanLists[NFREELISTS];
	static CentralCache _sInst;
};