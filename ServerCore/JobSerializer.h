// job, jobQueue�� �� �� ���� ����ϱ� ����
// job���� push�� pop�� ���� �����ؼ� ����ϰ� ����(Room.h����)
// �Ź� ������ ����� �������ϱ�
// JobQueue�� jobSerializer�� ��ӹްԲ� ������ֱ�

#pragma once
#include "Job.h"
#include "JobQueue.h"

/*------------------
	JobSerializer
-------------------*/
class JobSerializer : public enable_shared_from_this<JobSerializer>
{
public:
	// ���� ��� ����
	void PushJob(CallbackType&& callback)
	{
		// job������ֱ�
		// objectPool���� job�� ������ callback ���
		auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		_jobQueue.Push(job);
	}

	// �ι�° ����, �Ϲ������� �Լ� ����, ���ڵ� ���� �����ϴ� ���
	template<typename T, typename Ret, typename... Args>
	void PushJob(Ret(T::*memFunc)(Args...), Args... args)
	{
		// sharedptr�� ����Ѵٰ� ���ߴٸ�
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		_jobQueue.Push(job);
	}

	virtual void FlushJob() abstract;

protected:
	JobQueue _jobQueue;
};

