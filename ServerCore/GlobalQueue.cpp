#include "pch.h"
#include "GlobalQueue.h"

/*----------------
	GlobalQueue
------------------*/

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(JobQueueRef jobQueue)
{
	// 나중엔 lockQueue를 사용하지않고 다른 방식으로만들어도 됨
	// 하지만 지금은 그냥 있는 기능을 재사용
	_jobQueues.Push(jobQueue);
}

JobQueueRef GlobalQueue::Pop()
{
	return _jobQueues.Pop();
}
