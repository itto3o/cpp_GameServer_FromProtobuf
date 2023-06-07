#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

Room GRoom;

void Room::Enter(PlayerRef player)
{
	// players map�� playerId �߰�
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
		// �ϰ��� ���� ������ ������ ���鼭, ������ break;
		// �ϰ��� ���������� ȣ������ ��
		// (�ϰ��� ������ break�ǰ� return�ǰ����� ���� �����忡���� ���� ������ �ɾ����
		// ��� ������ �ִ� ��
		JobRef job = _jobs.Pop();
		if (job == nullptr)
			break;

		job->Execute();
	}
}
