#pragma once
class Room
{
	// ä�ù��� �����
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	USE_LOCK;
	map<uint64, PlayerRef> _players;
};

// roomManager�� ����� ������ �׽�Ʈ ���� ���� ��������
extern Room GRoom;

