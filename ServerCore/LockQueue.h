// ���־� �� �����ϱ� CorePch.h�� �߰�
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

	// ��� �ֵ��� �ѹ��� ������ �Լ�
	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
			items.push_back(item);
	}

	void Clear()
	{
		WRITE_LOCK;
		// �Ϻ� STL�� clear�� ���� ��찡 �ִµ�,
		// ���� ������ ����ִ� ť�� �ٽ� �о��ָ� ��
		//_items.clear();
		_items = Queue<T>();
	}

private:
	USE_LOCK;
	//queue<T> _items;
	Queue<T> _items; //STL allocator�� ����ϰԲ�
};

