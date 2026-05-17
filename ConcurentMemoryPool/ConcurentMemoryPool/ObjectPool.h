#pragma once
#include <iostream>
#include <vector>
using std::cout;
using std::endl;

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		if (_freelist)
		{
			void* next = *(void**)_freelist;
			obj = (T*)_freelist;
			_freelist = next;
		}
		else
		{
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 1028 * 128;
				_memory = (char*)malloc(_remainBytes);
				if (_memory == nullptr)
				{
					throw std::bad_alloc();
				}
			}

			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_remainBytes -= objSize;
		}

		new(obj)T;

		return obj;
	}

	void Delete(T* obj)
	{
		obj->~T();

		// 头插
		*(void**)obj = _freelist;
		_freelist = obj;
	}

private:
	char* _memory = nullptr;  // 指向大内存的指针
	size_t _remainBytes = 0;  // 指向大内存的在切分过程中的剩余字节数
	void* _freelist = nullptr;// 还回来过程中自由链表的指针

};

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 5;

	// 每轮申请释放多少次
	const size_t N = 100000;

	std::vector<TreeNode*> v1;
	v1.reserve(N);

	size_t begin1 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}

	size_t end1 = clock();

	std::vector<TreeNode*> v2;
	v2.reserve(N);

	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();

	cout << "new cost time:" << end1 - begin1 << endl;
	cout << "object pool cost time:" << end2 - begin2 << endl;
}