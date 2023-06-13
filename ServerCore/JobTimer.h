#pragma once

// 먼저 jobdAta를 만들어서 weakptr로 해당 잡을 실행해야될 오너를 들고 있음
struct JobData
{
	JobData(weak_ptr<JobQueue> owner, JobRef job) : owner(owner), job(job)
	{

	}
	// weakptr로 하는 이유는 job을 한참 후에 예약할 수도 있는데
	// 걔가 실행이 될때까지 shared로 물고 있으면
	// 생명 주기 상 그 시간동안은 소멸되지 못하게 될 수 있어서
	// 자주 체크하는 것도 아니고 실행할때 한번만 체크할거라
	weak_ptr<JobQueue>	owner;
	// 실행해야될 job들
	JobRef				job;
};

// 우선순위 큐에 들어갈 아이템 설계
struct TimerItem
{
	// 비교 오퍼레이터
	// 다른 애랑 비교해서 executeTick끼리 비교
	bool operator<(const TimerItem& other) const
	{
		return executeTick > other.executeTick; 
		// 우리가 만든 PriorityQueue는 less로 되어있어서
		// 큰거에서 작은 순으로 튀어나오게 되는데
		// 우리는 작은 시간부터 먼저 추출이 되길 원하기 때문에 (SLF인듯)
		// < 방향이 아닌 >으로 해야함
	}

	// uint64로 실행되어야 할 tick
	uint64 executeTick = 0;
	// 위에서 만들어준 jobData를 들고 있음
	// 포인터인 이유는 우선순위 큐라고 해도 그 위치가 알고리즘 특성상 바뀔수도 잇는데 
	// 그때마다 복사하면 스마트포인터 ref가 1늘고 1줄고 하는 부분에 영향을 줄 수도 있으니까
	// 어차피 이 포인터는 내부에서만 사용하고 나중에 해제를 해줄 것이기 때문에
	JobData* jobData = nullptr;
};

/*---------------
	JobTimer
----------------*/
// 전역으로 만들 예정
// --> CoreGlobal.h에 가서 선언
class JobTimer
{
public:
	// Reserve : jobtiemr에 예약을 한다
	void		Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job);
	
	// 일감들을 배분해서 원래 owner에 job을 꽂아주기
	void		Distribute(uint64 now);
	void		Clear();

private:
	USE_LOCK;
	PriorityQueue<TimerItem>	_items;
	// 얘를 이리저리 실행하고 있는지 
	// 다시 배치하고 있는지 여부,
	// ==> 한번에 한애만 일감을 맡겠다는 정책을 위함
	Atomic<bool>				_distributing = false;
};

