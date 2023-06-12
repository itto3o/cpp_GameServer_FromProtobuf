// job, jobQueue를 좀 더 쉽게 사용하기 위해
// job에서 push랑 pop을 각각 지정해서 사용하고 있음(Room.h에서)
// 매번 일일이 만들기 귀찮으니까
// JobQueue는 jobSerializer를 상속받게끔 만들어주기

#pragma once
#include "Job.h"
#include "LockQueue.h"

/*------------------
	JobQueue
-------------------*/
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	// 람다 사용 버전
	void DoAsync(CallbackType&& callback) //뭔가 비동기로 실행한다
	{
		// job만들어주기
		// objectPool에서 job을 꺼내서 callback 등록
		//auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
		//_jobs.Push(job);

		// 내가 처음으로 queue에 넣은 애라면 걔가 flush를 담당하게끔
	}

	// 두번째 버전, 일반적으로 함수 지정, 인자들 따로 지정하는 방식
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		// sharedptr을 사용한다고 정했다면
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		//auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
		//_jobs.Push(job);
	}

	//virtual void FlushJob() abstract;

	void ClearJobs() { _jobs.Clear(); }

private:
							// 오른값으로 job을 받아서 바로 move를 해서 밀어넣기
	void				Push(JobRef&& job);

public:
	void				Execute();

protected:
	LockQueue<JobRef>		_jobs;
	Atomic<int32>			_jobCount = 0;
};

