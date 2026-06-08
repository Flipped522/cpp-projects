#define _CRT_SECURE_NO_WARNINGS 1 

#include "ObjectPool.h"
#include "ConcurrentAlloc.h"

void Alloc1()
{
	for (size_t i = 0; i < 5; i++)
	{
		void* ptr = ConcurentAlloc(6);
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; i++)
	{
		void* ptr = ConcurentAlloc(6);
	}
}

void TLSTest()
{
	std::thread t1(Alloc1);
	t1.join();

	std::thread t2(Alloc2);
	t2.join();
}

void TestConcurrentAlloc1()
{
	void* p1 = ConcurentAlloc(6);
	void* p2 = ConcurentAlloc(8);
	void* p3 = ConcurentAlloc(1);
	void* p4 = ConcurentAlloc(7);
	void* p5 = ConcurentAlloc(8);

	cout << p1 << endl;
	cout << p2 << endl;
	cout << p3 << endl;
	cout << p4 << endl;
}

int main()
{
	TestConcurrentAlloc1();

	return 0;
}