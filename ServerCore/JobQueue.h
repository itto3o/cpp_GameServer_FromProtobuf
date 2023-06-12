// job, jobQueue�� �� �� ���� ����ϱ� ����
// job���� push�� pop�� ���� �����ؼ� ����ϰ� ����(Room.h����)
// �Ź� ������ ����� �������ϱ�
// JobQueue�� jobSerializer�� ��ӹްԲ� ������ֱ�

#pragma once
#include "Job.h"
#include "LockQueue.h"

/*------------------
	JobQueue
-------------------*/
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	// ���� ��� ����
	void DoAsync(CallbackType&& callback) //���� �񵿱�� �����Ѵ�
	{
		// job������ֱ�
		// objectPool���� job�� ������ callback ���
		//auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
		//_jobs.Push(job);

		// ���� ó������ queue�� ���� �ֶ�� �°� flush�� ����ϰԲ�
	}

	// �ι�° ����, �Ϲ������� �Լ� ����, ���ڵ� ���� �����ϴ� ���
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		// sharedptr�� ����Ѵٰ� ���ߴٸ�
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		//auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
		//_jobs.Push(job);
	}

	//virtual void FlushJob() abstract;

	void ClearJobs() { _jobs.Clear(); }

private:
							// ���������� job�� �޾Ƽ� �ٷ� move�� �ؼ� �о�ֱ�
	void				Push(JobRef&& job);

public:
	void				Execute();

protected:
	LockQueue<JobRef>		_jobs;
	Atomic<int32>			_jobCount = 0;
};

