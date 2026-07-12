#define _CRT_SECURE_NO_WARNINGS 1 
#pragma once

#include "Common.h"

class CentralCache
{
public:

private:
	SpanList _spanLists[NFREELISTS];
};