#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

Room GRoom;

void Room::Enter(PlayerRef player)
{
	// players map에 playerId 추가
	//WRITE_LOCK;
	_players[player->playerId] = player;
}

void Room::Leave(PlayerRef player)
{
	//WRITE_LOCK;
	_players.erase(player->playerId);
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	//WRITE_LOCK;
	for (auto& p : _players)
	{
		p.second->ownerSession->Send(sendBuffer);
	}
}

void Room::FlushJob()
{
	while (true)
	{
		// 일감이 있을 때까지 루프를 돌면서, 없으면 break;
		// 일감이 있을때마다 호출해줄 것
		// (일감이 없으면 break되고 return되겠지만 메인 스레드에서도 무한 루프를 걸어놔서
		// 계속 들어오고 있는 듯
		JobRef job = _jobs.Pop();
		if (job == nullptr)
			break;

		job->Execute();
	}
}
