#pragma once
#include "Job.h"

class Room
{
	friend class EnterJob;
	friend class LeaveJob;
	friend class BroadcastJob;
	// ä�ù��� �����

private:
	// �̱� ������ ȯ���� ���� �ڵ�
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

public:
	// ��Ƽ������� ���õ� �ֵ�, �ϰ����� ����
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob(); // �������� �ϰ��� ����, ������ �����ϰ� main�����尡 ���
	// �ϰ��� �о������ ���ÿ� �ƹ��� �����ϰ� ���� ������ �°� �����ϰų�
	// �ƴϸ� �и��ؼ� push�ϴ� �� ����, �����ϴ� �� ����

private:
//	USE_LOCK;
// jobQueue�� �̿��ϸ� ���������� LOCK�� ������� ����
	// ��� �׷� �� ������?
	// ==> �ִ��� job����� ����ϸ� enter, leave, broadcast���� ���������� �����ؼ� ȣ��x,
	// ������ �ϰ��� ���ؼ� ����, JobQueue�� �� �Լ����� WIRTE_LOCK�� �ɷ������ϱ�
	// �׳� ������ ª�� �ɾ��� ���� ��
	// ==> �⺻�Լ���(enter, leave, broadcast)�� �����ϴ°� �η����
	// private���� ����� friend class EnterJob;���� �����
	map<uint64, PlayerRef> _players;
	JobQueue _jobs;
};

// roomManager�� ����� ������ �׽�Ʈ ���� ���� ��������
extern Room GRoom;

// Room Job
class EnterJob : public IJob
{
public:
	EnterJob(Room& room, PlayerRef player) : _room(room), _player(player)
	{
	}

	virtual void Execute() override
	{
		_room.Enter(_player);
	}

public:
	Room& _room;
	PlayerRef _player;
};

class LeaveJob : public IJob
{
public:
	LeaveJob(Room& room, PlayerRef player) : _room(room), _player(player)
	{
	}

	virtual void Execute() override
	{
		_room.Leave(_player);
	}

public:
	Room& _room;
	PlayerRef _player;
};

class BroadcastJob : public IJob
{
public:
	BroadcastJob(Room& room, SendBufferRef sendBuffer) : _room(room), _sendBuffer(sendBuffer)
	{
	}

	virtual void Execute() override
	{
		_room.Broadcast(_sendBuffer);
	}

public:
	Room& _room;
	SendBufferRef _sendBuffer;
};