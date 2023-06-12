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
	// ���߿� lockQueue�� ��������ʰ� �ٸ� ������θ��� ��
	// ������ ������ �׳� �ִ� ����� ����
	_jobQueues.Push(jobQueue);
}

JobQueueRef GlobalQueue::Pop()
{
	return _jobQueues.Pop();
}
