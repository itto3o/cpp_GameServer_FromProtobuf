#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"
/*------------------
	JobQueue
-------------------*/

void JobQueue::Push(JobRef job, bool pushOnly)//(JobRef&& job)
{
	// atomic하게 jobCount를 1증가,
	// fetch add가 반환하는 값은 1증가하기 전의 값을 뱉어줌
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // 내부적으로 WRITE_LOCK을 잡아서 넣어주고 있음

	// 항상 count를 증가시킨 다음에 push를 하고
	// job을 실행한 다음에
	// count를 빼줘야함
	// --> 내가만약 처음에 온 스레드라 Execute를 실행하고 있따고 가정하면
	// 그 와중에 내가 실행하는 동안 다른 애가 일감을 밀어넣을 수 있음
	// --> 지금까지 쌓인 일감이 0개라면 실행한 후 count를 줄이는데
	// 그 사이에 일감을 추가해서 jobcount가 11개인데 아직 _jobs.Push가 실행되지 않아서
	// jobs에는 일감이 10개밖에 들어가있지 않다면
	// jobCount를 10을 빼면 0으로 떨어지지 않아서 returnㅇ르 하지 않고 다시 한바퀴 돌게 되는데
	// --> 먼저 count를 증가한후 push해야,
	// 일감을 추가해서 11개가 됐는데 count가 아직 10이라면 jobCount를 뺄때 -1이 나오게됨

	// count가 0이었다고 하면 내가 맨 처음 밀어넣게 되는 거라고 확신할 수 있음
	if (prevCount == 0)
	{
		// prevCount가 0이면 항상 실행했지만 이제는 아님
		// 먼저 조건 체크, 이미 실행중인 jobQUeue가 없으면 실행
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			// 실행 담당
			Execute();
		}
		else
		{
			// 이미 담당하는 애가 있다면 다른 애한테 떠넘김
			// globalQueue에 등록,
			// 여유있는 다른 스레드가 실행하도록
			GGlobalQueue->Push(shared_from_this());
			// 누군가는 꺼내서 실행을 해야할텐데, 이걸 어떻게 배분해야할까?
			// --> ThreadManager.h에 가서 DoGlobalQueueWork() 함수 만들기
			// 이 함수를 누군가 호출해서 실행해야하는데, 이거는 GameServer.cpp에서 살펴봄
		}
	}
}

void JobQueue::Execute()
{
	// Execute로 들어왔다는건 내가 얘를 담당하겠다는 거니까 this로 가리킴
	LCurrentJobQueue = this;

	// 루프를 돌면서 하나씩 실행
	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs); //jobs에 _jobs를 모두 꺼냄

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute(); // job.h의 Execute가 실행되네 ㄷㄷ

		// fetch _ sub, 빼기 이전의 값이 나옴
		// 이 값이 jobCount만큼이었다고 하면 딱 0일 것임
		if (_jobCount.fetch_sub(jobCount) == jobCount) //원래 값이 내가 빼준값이랑 동일하다면
		{
			// 남은 일감이 0개라면 종료
			LCurrentJobQueue = nullptr; //다시 null로 밀어서 호출이 끝남
			return;
		}

		// 시간을 체크해서 현재시간이 원래 할당받은 시간보다 더 크다고 하면 그냥 나오기
		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}
