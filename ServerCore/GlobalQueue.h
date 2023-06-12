#pragma once

/*----------------
	GlobalQueue
------------------*/
// jobQueue를 전역으로 들고있는 애
// --> CoreGlobal.h에 추가
// 현재 스레드가 jobQueue를 관리하고 있는지 체크하기 위한 변수를
// TLS에 (CoreTLS.h)
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	//jobQUeue의 스마트 포인터를 들고 있도록
	// JobQueueRef는 types.h에 가서 추가
	// Job이 아니라 JobQUeue를 들고와서 다른 애가 jobQueue를 이어받아서 걔를 실행할 수 있게
	void					Push(JobQueueRef jobQueue);
	JobQueueRef				Pop();

private:
	//LockQueue의 JobQueueRef타입을 들고 있도록
	// LockQueue는 JobQueueRef 타입을 받아서 직접 lock을 걸고 push, pop하는 애
	LockQueue<JobQueueRef>	_jobQueues;
};

