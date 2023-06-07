#pragma once
// 인터페이스 만들기
class IJob
{
public:
	virtual void Execute() { }
};

// 위의 인터페이스를 상속을 받아서 일을 만들것
class HealJob : public IJob
{
public:
	virtual void Execute() override
	{
		// _target을 찾아서
		// _target0->AddHP(_healValue);
		cout << _target << "한테 힐" << _healValue << " 만큼 줌";
	}
public:
	// heal을 1번 유저한테 줘야하니까
	// 인자들을 같이 들고 있어야함
	uint64 _target = 0;
	uint32 _healValue = 0;
};

// jobQueue로 순차적 처리할 수 있게끔
using JobRef = shared_ptr<IJob>;

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
