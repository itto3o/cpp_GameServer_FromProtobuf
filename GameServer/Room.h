#pragma once
#include "Job.h"

class Room
{
	friend class EnterJob;
	friend class LeaveJob;
	friend class BroadcastJob;
	// 채팅방을 얘기함

private:
	// 싱글 쓰레드 환경인 마냥 코딩
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

public:
	// 멀티스레드와 관련된 애들, 일감으로 접근
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob(); // 누군가는 일감을 실행, 오늘은 간단하게 main스레드가 담당
	// 일감을 밀어넣음과 동시에 아무도 실행하고 있지 않으면 걔가 실행하거나
	// 아니면 분리해서 push하는 애 따로, 실행하는 애 따로

private:
//	USE_LOCK;
// jobQueue를 이용하면 내부적으로 LOCK은 사용하지 않음
	// 어떻게 그럴 수 있을까?
	// ==> 애당초 job방식을 사용하면 enter, leave, broadcast들은 직접적으로 접근해서 호출x,
	// 무조건 일감을 통해서 접근, JobQueue의 각 함수에는 WIRTE_LOCK이 걸려있으니까
	// 그냥 굉장히 짧게 걸어준 셈이 됨
	// ==> 기본함수들(enter, leave, broadcast)에 접근하는게 두려우면
	// private으로 만들고 friend class EnterJob;으로 만들기
	map<uint64, PlayerRef> _players;
	JobQueue _jobs;
};

// roomManager를 만들기 귀찮아 테스트 용을 위한 전역변수
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