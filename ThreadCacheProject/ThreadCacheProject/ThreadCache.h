#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once
#include "Common.h"

class ThreadCache
{
public:
	// 申请和释放内存对象
	void* Allocate(size_t size);

	void Dellocate(void* ptr, size_t size);
private:
	FreeList _freeLists[];
};