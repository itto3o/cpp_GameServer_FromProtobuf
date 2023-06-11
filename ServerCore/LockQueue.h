// 자주쓸 것 같으니까 CorePch.h에 추가
#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WRITE_LOCK;
		_items.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;

	}

	// 모든 애들을 한번에 추출할 함수
	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
			items.push_back(item);
	}

	void Clear()
	{
		WRITE_LOCK;
		// 일부 STL은 clear가 없는 경우가 있는데,
		// 만약 없으면 비어있는 큐로 다시 밀어주면 됨
		//_items.clear();
		_items = Queue<T>();
	}

private:
	USE_LOCK;
	//queue<T> _items;
	Queue<T> _items; //STL allocator를 사용하게끔
};

