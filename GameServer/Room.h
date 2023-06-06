#pragma once
class Room
{
	// 채팅방을 얘기함
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	USE_LOCK;
	map<uint64, PlayerRef> _players;
};

// roomManager를 만들기 귀찮아 테스트 용을 위한 전역변수
extern Room GRoom;

