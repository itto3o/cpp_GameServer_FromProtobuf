#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "GameSession.h"
//#include "BufferReader.h"
//#include "BufferWriter.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];

//void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
//{
//	BufferReader br(buffer, len);
//
//	PacketHeader header;
//	br.Peek(&header);
//
//	// map이나 배열로 만들어서 index하게 만들어줘도 됨(메모리 낭비는 있겠지만)
//	switch (header.id)
//	{
//	default:
//		break;
//	}
//}

//SendBufferRef ServerPacketHandler::MakeSendBuffer(Protocol::S_TEST& pkt)
//{
//	return _MakeSendBuffer(pkt, S_TEST);
//}

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//TODO : Log
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	// 클라에서 로그인 요청이 오면 처리해주는 부분
	// 패킷 세션 ref로 받아주고 있지만 정보 자체는 gameSession에 다 있으니까 그걸로 casting
	// gamesessionRef가 없으니까 pch.h 에 추가
	// 나중엔 clientSEssion으로 하는게 좋음
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : Validation 체크
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	// DB에서 플레이어 정보를 글겅옴
	// Gamesession에 플레이 정보를 저장(메모리에)
	// 보통은 계정과 관련된 account클래스를 만들어서 거기에 저장하는게 일반적

	// ID 발급
	static Atomic<uint64> idGenerator = 1;

	{
		// 패킷을 채우는 작업, 메모리 상에도 들고있어야함
		auto player = loginPkt.add_players();
		player->set_name(u8"DB에서긁어온이름1");
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		// proto에서 만들어준 player 자체로 들고 있어도 되는데 Player클래스를 만들어놨으니까 그거 응용
		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		// 메모리 상으로 들고있긴 해야함
		gameSession->_players.push_back(playerRef);
	}

	{
		auto player = loginPkt.add_players();
		player->set_name(u8"DB에서긁어온이름2");
		player->set_playertype(Protocol::PLAYER_TYPE_MAGE);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.push_back(playerRef);
	}

	// 다 만들었으면 sendBuffer를 통해 보내기
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pkt.playerindex();
	// TODO : validation

	// 위에서 메모리에 들고있어야 하는 이유
	// 매번 DB에서 긁어오지 않고 player정보가 현재 gameSession에 있을 테니까
	// 메모리에서 들고 있으면 바로 꺼내쓸 수 있게

	// 그런데 멀티스레드 환경에서 과연 안전한가?
	// 지금 단계에서는 문제는 없음
	// 플레이어 세팅하는 부분이 Login부분이라 인게임에서 플레이어를 추가하는 일은 없으니까
	// LOGIN이 맨 처음 게임 켜졌을 때 한번만 발생할 것이다? 는 굉장히 위험한 생각
	// 서버는 정상적인 상황만 생각하면 안됨,
	// login패킷을 조작해서 언제어디서든 올 수 있다고 생각해서 validation을 체크해야함
	// (로비에서만 login할 수 있다던지)
	PlayerRef player = gameSession->_players[index]; // READ_ONLY?
	//GRoom.Enter(player);
	// job을 만들어서 호출해줘야함
	//GRoom.PushJob(MakeShared<EnterJob>(GRoom, player)); // 일감만 예약
	//GRoom.PushJob(&Room::Enter, player);
	//GRoom->PushJob(&Room::Enter, player);
	GRoom->DoAsync(&Room::Enter, player);
	// ==> 실행은 안 된 상태라 밑에서 pkt를 만드는게 좀 이상해 보임
	// ==> 밑의 부분을 Enter가 실행된 후에 만들어지도록 옮겨주거나 바꿔줘야
	// 핵심은 아니라 넘어가기

	Protocol::S_ENTER_GAME enterGamePkt;
	enterGamePkt.set_success(true);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	player->ownerSession->Send(sendBuffer);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	//GRoom.Broadcast(sendBuffer); // 내부적으로는WRITE_LOCK을 잡고 있음,
	//GRoom.PushJob(MakeShared<BroadcastJob>(GRoom, sendBuffer));
	//GRoom.PushJob(&Room::Broadcast, sendBuffer);
	//GRoom->PushJob(&Room::Broadcast, sendBuffer);
	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}

//bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt)
//{
//	// TODO : 진짜 컨텐츠 작업
//	return true;
//}
//
//bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
//{
//	//TODO
//	return true;
//}
