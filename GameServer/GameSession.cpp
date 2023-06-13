#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	// ���⼭ Ȯ���� �������
	// currentPlayer�� null���
	if (_currentPlayer)
	{
		// �浵 ��������, �������� ����, weakptr�� ��������ϱ� .lock()�� ���ؼ� sharedptr�� ��ȯ�� ������
		if (auto room = _room.lock())
			// null�� �ƴ϶�� DoAsync�� �ؼ� ����(currentPlayer�� �Ҹ���Ѵ޶��)
			room->DoAsync(&Room::Leave, _currentPlayer);
	}

	// refCount ����
	_currentPlayer = nullptr;
	_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	// ��Ŷ���Ƿ����� �ڽ��� ������ �ǳ��� �� ���ڷ� �Ѱ��ֱ�
	PacketSessionRef session = GetPacketSessionRef();

	// header�ѹ� �� üũ, ���� ������ ���� �� �����ϱ� �뿪��üũ �� �и������
	// DB, ���Ӽ������� �Ѹ� ����� �Ŷ�� ��Ŷ���踦�س��µ� ���߿� Ŭ����ŷ�ؼ� �� �����U����
	// �ǳ��ְԵǸ� �׷� �κе� �Ÿ� �� �־�� ��, ������ Ŭ��� ���õ� �κи� �������ֵ���
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : PacketId �뿪 üũ
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}