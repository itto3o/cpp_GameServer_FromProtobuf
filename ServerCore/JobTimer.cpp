#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/*---------------
	JobTimer
----------------*/
void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	// ���� ����ž� �ϴ� executeTick�� ���, �� �� �Ŀ� reserve�� �ϰڴٴ� ������ �־�����ϱ�
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	// jobData�����
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job); //��� lock�� �ɷ�����

	// �������� ���Ǿ�� �ϱ� ������ WRITE_LOCK�� �ɱ�
	WRITE_LOCK;

	// items�� push
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	// �ѹ��� �� �����常 ���
	// ��? �ؿ��� LOCK�� ��� �ִµ�?
	// ==> �ؾ��� ��Ȳ����, ������ ���̷� �ϰ��� �ΰ��� �ִٰ� �����ϸ�
	// ���� �����尡 ���� �����ؼ� �������ٸ�
	// �� �ְ� ù��° �ϰ��� ���� ������ �¸� �о�������� �� ����
	// �ٸ� �ְ� �� ������ �κ��� �����ؼ� ������ �ι�°, ����° �ϰ��� ������ �°� �ռ� ����ȴٰų�
	// �ϰ� ������ ���� ���� �ֱ� ������
	
	// �ٸ��ְ� distribute�۾��� �ϰ������� ����������
	// exchange�� true��, �������� true���ٰ� �ϸ� ������ �ϰ� �ִٴ� �Ŵϱ� return
	if (_distributing.exchange(true) == true)
		return;

	// itmes��ü�� lock�� ��Ƽ� �ٸ� ������ ������ �� �ֱ� ������
	// �ִ��� ������ item���� �� ������ �۾��� �ؾ�
	// => �ּ������� ���� ��� ���� �ӽ� ���͸� ���� items�� �� ����
	// lock�� ��� ����Ǿ�� �� �ֵ鸸 ������ ��
	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		// item�� �ִµ��� ����
		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();

			// ���� �ð��� executeTick���� ������ ���� ���� �ƴ�, break,
			if (now < timerItem.executeTick)
				break; //�ٵ� �� continue�� �ƴѰɱ�

			// �װ� �ƴ϶�� �����Ҷ��� �ƴٴ°Ŵϱ� timeritem�� vector�� �о�ֱ�
			items.push_back(timerItem);
			_items.pop(); // �켱���� ť���� ��������
		}
	}

	// ���⼭���� �̱۽�����ϱ� lock ��� ��
	// owwenr�� �ٽ� �����ؼ� lock�� �̿��ؼ� sharedptr�� �ٲ㼭 owner�� null�� �ƴϸ�
	// owner�� push�ؼ� jobdata�� ������ �о�־��ְ�
	// objectPool�� push�ؼ� item.jobData�� �Ҹ�
	for (TimerItem& item : items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job, true); //true�� �־�� ���� ������?
											// --> true�� �´� �Ѵ� �غ���!
											// false�϶��� 1000 2000 Helloworld 3000 helloworld���µ�
											// true�� �ٲٴϱ� 1000 helloworld 2000 helloworld 3000
											// �̷������� ���, ������ �ð��� 1�� 2�� 3�� �ɸ��� �����̶�
											// true�� Ȯ���� �´µ�
			// �������� push�ϸ� ó���� ���� �¸� �����ϴ� �ſ��µ�
		// ������ �׳� ���� ����ϴ� ���Ҹ� �ϱ� ���ؼ� option�� �ϳ� �� �ֱ� push�� �ι�° ���ڷ�

		ObjectPool<JobData>::Push(item.jobData);
	}

	// �������� Ǯ���ֱ�
	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	// item�� �ϳ��� ���� ������
	while (_items.empty() == false)
	{
		// item�� ����
		const TimerItem& timerItem = _items.top();

		// objectpool�� push�ؼ� ��ȯ
		ObjectPool<JobData>::Push(timerItem.jobData);

		// items�� pop�� �ؼ� �ϰ� �����ֱ�
		_items.pop();
	}
}
