// 자주쓸 것 같으니까 CorePch.h에 추가
#pragma once

class JobQueue
{
public:
	void Push(JobRef job)
	{
		WRITE_LOCK;
		_jobs.push(job);
	}

	JobRef Pop()
	{
		WRITE_LOCK;
		if (_jobs.empty())
			return nullptr;

		JobRef ret = _jobs.front();
		_jobs.pop();
		return ret;

	}

private:
	USE_LOCK;
	queue<JobRef> _jobs;
};

