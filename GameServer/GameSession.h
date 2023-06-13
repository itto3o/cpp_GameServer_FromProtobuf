#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
	~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

	// 자기가 들고있는 플레이어도 메모리 상으로 들고 있긴 해야함
public:
	Vector<PlayerRef>	_players;

	PlayerRef			_currentPlayer;
	// room같은 경우는 없을수도 있으니 weakptr, sharedptr도 가능
	weak_ptr<class Room> _room;
	// 포인터가 마음에 안들면 아이디를 들고있게 해서
	// id를 이용해서 딕셔너리, 해시테이블 같은 자료구조로 빼오는 것도 방법
};