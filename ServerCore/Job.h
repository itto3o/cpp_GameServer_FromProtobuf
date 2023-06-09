#pragma once
#include <functional>

// C++ 11부터 lamda식을 functional과 조합
// 스마트 포인터와 관련된 실수를 안할 보장이 없으니까 mapping시켜서 관리
/*-------------------
		Job
--------------------*/
using CallbackType = std::function<void()>; // 람다 타입이 인자를 받는건 아니고 
// 내부적으로 캡처해서 인자들을 복사하는 거기 때문에 람다 자체만 보면 void void가 맞음

class Job
{
public:

	// 생성자에서 callback을 넣어줫따면 막바로 callback을 옮겨주기
	Job(CallbackType&& callback) : _callback(std::move(callback))
	{
	}

	// sharedptr을 쓰는게 너무 귀찮다면 여러가지 버전으로 만들어주기
	// job을 만들때 오너, 멤버함수, 인자들을 같이 넣어주면 알아서 스마트포인터로 동작
	// sharedptr을 건네준 후 함수는 대부분 멤버함수로 사용할거라 이 버전을 특별히 챙겨줌 
	// && :보편참조로 만들어서 건네주기
	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
					// 자기 자신 복사해서 refCount +1
		_callback = [owner, memFunc, args...]()
		{
			// get으로 포인터를 꺼내와서 거기에 멤버함수를 호출하고 인자들은args...
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

