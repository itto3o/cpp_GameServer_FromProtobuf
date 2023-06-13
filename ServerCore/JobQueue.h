// job, jobQueue�� �� �� ���� ����ϱ� ����
// job���� push�� pop�� ���� �����ؼ� ����ϰ� ����(Room.h����)
// �Ź� ������ ����� �������ϱ�
// JobQueue�� jobSerializer�� ��ӹްԲ� ������ֱ�

#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

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

	// ������ �ʿ��� ����� ����
	void DoTimer(uint64 tickAfter, CallbackType&& callback) 
	{
		//Push(ObjectPool<Job>::MakeShared(std::move(callback)));
		// ���� push�ϸ鼭 job�� ��������� �¸� �ٷ� jobQueue�� �Ⱦ���µ�
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		// timer�� reserve�� �ؼ� tickafter�� sharedfromthis(���ʴ� ��) ����
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);

	}

	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		//Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
		// �̷��� ������ �ڿ� �������� �ٽ� ������ distribute, ������ ã�Ƽ� push
	}

	//virtual void FlushJob() abstract;

	void ClearJobs() { _jobs.Clear(); }

//private:
							// ���������� job�� �޾Ƽ� �ٷ� move�� �ؼ� �о�ֱ�
	//void				Push(JobRef&& job); // ��� �׳� ����, �׷��Ա��� �Ű澵 �ʿ�� ����
											// �����ϴ��� refCount 1����, 1�����ϴ� �� ����
public:
	//void				Push(JobRef job);
	void				Push(JobRef job, bool pushOnly = false); //pushOnly�� true��� 
											//execute���� �ʰ� �۷ι�ť�� �ְ� ����������
											// ��и� �Ϸ��� �ǵ���� �۷ι�ť���� ��������
	void				Execute();

protected:
	LockQueue<JobRef>		_jobs;
	Atomic<int32>			_jobCount = 0;
};

