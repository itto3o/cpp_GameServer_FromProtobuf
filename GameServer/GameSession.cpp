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

	// 여기서 확실히 끊어줘야
	// currentPlayer가 null라면
	if (_currentPlayer)
	{
		// 방도 있을수도, 없을수도 있음, weakptr로 들고있으니까 .lock()을 통해서 sharedptr로 변환한 다음에
		if (auto room = _room.lock())
			// null이 아니라면 DoAsync를 해서 예약(currentPlayer를 소멸시켜달라는)
			room->DoAsync(&Room::Leave, _currentPlayer);
	}

	// refCount 감소
	_currentPlayer = nullptr;
	_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	// 패킷세션레프로 자신이 누군지 건네준 후 인자로 넘겨주기
	PacketSessionRef session = GetPacketSessionRef();

	// header한번 더 체크, 여러 서버가 있을 수 있으니까 대역대체크 후 분리해줘야
	// DB, 게임서버에서 둘만 사용할 거라고 패킷설계를해놨는데 나중에 클라해킹해서 그 서버쪾으로
	// 건네주게되면 그런 부분도 거를 수 있어야 함, 정말로 클라랑 관련된 부분만 실행해주도록
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : PacketId 대역 체크
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}