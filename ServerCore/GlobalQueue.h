#pragma once

/*----------------
	GlobalQueue
------------------*/
// jobQueue�� �������� ����ִ� ��
// --> CoreGlobal.h�� �߰�
// ���� �����尡 jobQueue�� �����ϰ� �ִ��� üũ�ϱ� ���� ������
// TLS�� (CoreTLS.h)
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	//jobQUeue�� ����Ʈ �����͸� ��� �ֵ���
	// JobQueueRef�� types.h�� ���� �߰�
	// Job�� �ƴ϶� JobQUeue�� ���ͼ� �ٸ� �ְ� jobQueue�� �̾�޾Ƽ� �¸� ������ �� �ְ�
	void					Push(JobQueueRef jobQueue);
	JobQueueRef				Pop();

private:
	//LockQueue�� JobQueueRefŸ���� ��� �ֵ���
	// LockQueue�� JobQueueRef Ÿ���� �޾Ƽ� ���� lock�� �ɰ� push, pop�ϴ� ��
	LockQueue<JobQueueRef>	_jobQueues;
};

