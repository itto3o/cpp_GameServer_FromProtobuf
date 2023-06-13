#pragma once

// ���� jobdAta�� ���� weakptr�� �ش� ���� �����ؾߵ� ���ʸ� ��� ����
struct JobData
{
	JobData(weak_ptr<JobQueue> owner, JobRef job) : owner(owner), job(job)
	{

	}
	// weakptr�� �ϴ� ������ job�� ���� �Ŀ� ������ ���� �ִµ�
	// �°� ������ �ɶ����� shared�� ���� ������
	// ���� �ֱ� �� �� �ð������� �Ҹ���� ���ϰ� �� �� �־
	// ���� üũ�ϴ� �͵� �ƴϰ� �����Ҷ� �ѹ��� üũ�ҰŶ�
	weak_ptr<JobQueue>	owner;
	// �����ؾߵ� job��
	JobRef				job;
};

// �켱���� ť�� �� ������ ����
struct TimerItem
{
	// �� ���۷�����
	// �ٸ� �ֶ� ���ؼ� executeTick���� ��
	bool operator<(const TimerItem& other) const
	{
		return executeTick > other.executeTick; 
		// �츮�� ���� PriorityQueue�� less�� �Ǿ��־
		// ū�ſ��� ���� ������ Ƣ����� �Ǵµ�
		// �츮�� ���� �ð����� ���� ������ �Ǳ� ���ϱ� ������ (SLF�ε�)
		// < ������ �ƴ� >���� �ؾ���
	}

	// uint64�� ����Ǿ�� �� tick
	uint64 executeTick = 0;
	// ������ ������� jobData�� ��� ����
	// �������� ������ �켱���� ť��� �ص� �� ��ġ�� �˰��� Ư���� �ٲ���� �մµ� 
	// �׶����� �����ϸ� ����Ʈ������ ref�� 1�ð� 1�ٰ� �ϴ� �κп� ������ �� ���� �����ϱ�
	// ������ �� �����ʹ� ���ο����� ����ϰ� ���߿� ������ ���� ���̱� ������
	JobData* jobData = nullptr;
};

/*---------------
	JobTimer
----------------*/
// �������� ���� ����
// --> CoreGlobal.h�� ���� ����
class JobTimer
{
public:
	// Reserve : jobtiemr�� ������ �Ѵ�
	void		Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job);
	
	// �ϰ����� ����ؼ� ���� owner�� job�� �Ⱦ��ֱ�
	void		Distribute(uint64 now);
	void		Clear();

private:
	USE_LOCK;
	PriorityQueue<TimerItem>	_items;
	// �긦 �̸����� �����ϰ� �ִ��� 
	// �ٽ� ��ġ�ϰ� �ִ��� ����,
	// ==> �ѹ��� �Ѿָ� �ϰ��� �ðڴٴ� ��å�� ����
	Atomic<bool>				_distributing = false;
};

