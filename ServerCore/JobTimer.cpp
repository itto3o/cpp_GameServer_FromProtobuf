#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/*---------------
	JobTimer
----------------*/
void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	// 먼저 실행돼야 하는 executeTick을 계산, 몇 초 후에 reserve를 하겠다는 식으로 넣어놨으니까
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	// jobData만들기
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job); //얘는 lock이 걸려있음

	// 전역에서 사용되어야 하기 때문에 WRITE_LOCK을 걸기
	WRITE_LOCK;

	// items에 push
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	// 한번에 한 스레드만 통과
	// 왜? 밑에서 LOCK을 잡고 있는데?
	// ==> 극악의 상황에서, 간발의 차이로 일감이 두개가 있다고 생각하면
	// 여러 스레드가 동시 접근해서 꺼내쓴다면
	// 한 애가 첫번째 일감을 꺼낸 다음에 걔를 밀어넣으려는 그 순간
	// 다른 애가 또 나머지 부분을 실행해서 나머지 두번째, 세번째 일감을 꺼내서 걔가 앞서 실행된다거나
	// 일감 순서가 꼬일 수도 있기 때문에
	
	// 다른애가 distribute작업을 하고있으면 빠져나오게
	// exchange를 true로, 이전값이 true였다고 하면 누군가 하고 있다는 거니까 return
	if (_distributing.exchange(true) == true)
		return;

	// itmes자체는 lock을 잡아서 다른 누군가 접근할 수 있기 때문에
	// 최대한 빠르게 item들을 다 꺼내서 작업을 해야
	// => 최소한으로 락을 잡기 위해 임시 벡터를 만들어서 items를 다 복사
	// lock을 잡고 실행되어야 할 애들만 빼오게 됨
	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		// item이 있는동안 실행
		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();

			// 현재 시간이 executeTick보다 작으면 아직 때가 아님, break,
			if (now < timerItem.executeTick)
				break; //근데 왜 continue가 아닌걸까

			// 그게 아니라면 실행할때가 됐다는거니까 timeritem을 vector에 밀어넣기
			items.push_back(timerItem);
			_items.pop(); // 우선순위 큐에서 빠져나옴
		}
	}

	// 여기서부턴 싱글스레드니까 lock 없어도 됨
	// owwenr를 다시 추출해서 lock을 이용해서 sharedptr로 바꿔서 owner가 null이 아니면
	// owner에 push해서 jobdata를 꺼내서 밀어넣어주고
	// objectPool에 push해서 item.jobData를 소멸
	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job, true); //true를 넣어야 하지 않을까?
											// --> true가 맞대 둘다 해보자!
											// false일때는 1000 2000 Helloworld 3000 helloworld였는데
											// true로 바꾸니까 1000 helloworld 2000 helloworld 3000
											// 이런식으로 뜬당, 실제도 시간도 1초 2초 3초 걸리는 느낌이라
											// true가 확실히 맞는듯
			// 기존에는 push하면 처음에 들어가면 걔를 실행하는 거였는데
		// 지금은 그냥 일을 배분하는 역할만 하기 위해서 option을 하나 더 주기 push에 두번째 인자로

		ObjectPool<JobData>::Push(item.jobData);
	}

	// 끝났으면 풀어주기
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	// item에 하나라도 남을 때까지
	while (_items.empty() == false)
	{
		// item을 꺼냄
		const TimerItem& timerItem = _items.top();

		// objectpool에 push해서 반환
		ObjectPool<JobData>::Push(timerItem.jobData);

		// items에 pop을 해서 일감 꺼내주기
		_items.pop();
	}
}
