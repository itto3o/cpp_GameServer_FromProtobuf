// job, jobQueue를 좀 더 쉽게 사용하기 위해
// job에서 push랑 pop을 각각 지정해서 사용하고 있음(Room.h에서)
// 매번 일일이 만들기 귀찮으니까
// JobQueue는 jobSerializer를 상속받게끔 만들어주기

#pragma once
#include "Job.h"
#include "JobQueue.h"

/*------------------
	JobSerializer
-------------------*/
class JobSerializer : public enable_shared_from_this<JobSerializer>
{
public:
	// 람다 사용 버전
	void PushJob(CallbackType&& callback)
	{
		// job만들어주기
		// objectPool에서 job을 꺼내서 callback 등록
		auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		_jobQueue.Push(job);
	}

	// 두번째 버전, 일반적으로 함수 지정, 인자들 따로 지정하는 방식
	template<typename T, typename Ret, typename... Args>
	void PushJob(Ret(T::*memFunc)(Args...), Args... args)
	{
		// sharedptr을 사용한다고 정했다면
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		_jobQueue.Push(job);
	}

	virtual void FlushJob() abstract;

protected:
	JobQueue _jobQueue;
};

