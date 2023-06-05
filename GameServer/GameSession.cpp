#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
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