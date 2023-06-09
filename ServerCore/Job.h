#pragma once
#include <functional>

// C++ 11���� lamda���� functional�� ����
// ����Ʈ �����Ϳ� ���õ� �Ǽ��� ���� ������ �����ϱ� mapping���Ѽ� ����
/*-------------------
		Job
--------------------*/
using CallbackType = std::function<void()>; // ���� Ÿ���� ���ڸ� �޴°� �ƴϰ� 
// ���������� ĸó�ؼ� ���ڵ��� �����ϴ� �ű� ������ ���� ��ü�� ���� void void�� ����

class Job
{
public:

	// �����ڿ��� callback�� �־�Z���� ���ٷ� callback�� �Ű��ֱ�
	Job(CallbackType&& callback) : _callback(std::move(callback))
	{
	}

	// sharedptr�� ���°� �ʹ� �����ٸ� �������� �������� ������ֱ�
	// job�� ���鶧 ����, ����Լ�, ���ڵ��� ���� �־��ָ� �˾Ƽ� ����Ʈ�����ͷ� ����
	// sharedptr�� �ǳ��� �� �Լ��� ��κ� ����Լ��� ����ҰŶ� �� ������ Ư���� ì���� 
	// && :���������� ���� �ǳ��ֱ�
	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
					// �ڱ� �ڽ� �����ؼ� refCount +1
		_callback = [owner, memFunc, args...]()
		{
			// get���� �����͸� �����ͼ� �ű⿡ ����Լ��� ȣ���ϰ� ���ڵ���args...
			(owner.get()->*memFunc)(args...);
		};
	}

	void Execute()
	{
		_callback();
	}

private:
	CallbackType _callback;
};

