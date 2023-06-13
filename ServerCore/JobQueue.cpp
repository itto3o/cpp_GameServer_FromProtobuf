#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"
/*------------------
	JobQueue
-------------------*/

void JobQueue::Push(JobRef job, bool pushOnly)//(JobRef&& job)
{
	// atomic�ϰ� jobCount�� 1����,
	// fetch add�� ��ȯ�ϴ� ���� 1�����ϱ� ���� ���� �����
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // ���������� WRITE_LOCK�� ��Ƽ� �־��ְ� ����

	// �׻� count�� ������Ų ������ push�� �ϰ�
	// job�� ������ ������
	// count�� �������
	// --> �������� ó���� �� ������� Execute�� �����ϰ� �ֵ��� �����ϸ�
	// �� ���߿� ���� �����ϴ� ���� �ٸ� �ְ� �ϰ��� �о���� �� ����
	// --> ���ݱ��� ���� �ϰ��� 0����� ������ �� count�� ���̴µ�
	// �� ���̿� �ϰ��� �߰��ؼ� jobcount�� 11���ε� ���� _jobs.Push�� ������� �ʾƼ�
	// jobs���� �ϰ��� 10���ۿ� ������ �ʴٸ�
	// jobCount�� 10�� ���� 0���� �������� �ʾƼ� return���� ���� �ʰ� �ٽ� �ѹ��� ���� �Ǵµ�
	// --> ���� count�� �������� push�ؾ�,
	// �ϰ��� �߰��ؼ� 11���� �ƴµ� count�� ���� 10�̶�� jobCount�� ���� -1�� �����Ե�

	// count�� 0�̾��ٰ� �ϸ� ���� �� ó�� �о�ְ� �Ǵ� �Ŷ�� Ȯ���� �� ����
	if (prevCount == 0)
	{
		// prevCount�� 0�̸� �׻� ���������� ������ �ƴ�
		// ���� ���� üũ, �̹� �������� jobQUeue�� ������ ����
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			// ���� ���
			Execute();
		}
		else
		{
			// �̹� ����ϴ� �ְ� �ִٸ� �ٸ� ������ ���ѱ�
			// globalQueue�� ���,
			// �����ִ� �ٸ� �����尡 �����ϵ���
			GGlobalQueue->Push(shared_from_this());
			// �������� ������ ������ �ؾ����ٵ�, �̰� ��� ����ؾ��ұ�?
			// --> ThreadManager.h�� ���� DoGlobalQueueWork() �Լ� �����
			// �� �Լ��� ������ ȣ���ؼ� �����ؾ��ϴµ�, �̰Ŵ� GameServer.cpp���� ���캽
		}
	}
}

void JobQueue::Execute()
{
	// Execute�� ���Դٴ°� ���� �긦 ����ϰڴٴ� �Ŵϱ� this�� ����Ŵ
	LCurrentJobQueue = this;

	// ������ ���鼭 �ϳ��� ����
	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs); //jobs�� _jobs�� ��� ����

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute(); // job.h�� Execute�� ����ǳ� ����

		// fetch _ sub, ���� ������ ���� ����
		// �� ���� jobCount��ŭ�̾��ٰ� �ϸ� �� 0�� ����
		if (_jobCount.fetch_sub(jobCount) == jobCount) //���� ���� ���� ���ذ��̶� �����ϴٸ�
		{
			// ���� �ϰ��� 0����� ����
			LCurrentJobQueue = nullptr; //�ٽ� null�� �о ȣ���� ����
			return;
		}

		// �ð��� üũ�ؼ� ����ð��� ���� �Ҵ���� �ð����� �� ũ�ٰ� �ϸ� �׳� ������
		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}
