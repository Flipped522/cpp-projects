#define _CRT_SECURE_NO_WARNINGS 1 
#include <iostream>
#include <vector>
#include <time.h>
#include <thread>
#include <assert.h>
using std::cout;
using std::endl;

static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

// 管理切分的自由链表
class FreeList
{
public:
	void Push(void* obj)
	{
		NextObj(obj) = _freelist;
		_freelist = obj;
	}

	void* Pop()
	{
		assert(_freelist);
		void* obj = _freelist;
		_freelist = NextObj(obj);
		return obj;
	}

	bool Empty()
	{
		return _freelist == nullptr;
	}
private:
	void* _freelist = nullptr;
};