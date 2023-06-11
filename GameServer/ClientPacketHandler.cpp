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
//	// map�̳� �迭�� ���� index�ϰ� ������൵ ��(�޸� ����� �ְ�����)
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
	// Ŭ�󿡼� �α��� ��û�� ���� ó�����ִ� �κ�
	// ��Ŷ ���� ref�� �޾��ְ� ������ ���� ��ü�� gameSession�� �� �����ϱ� �װɷ� casting
	// gamesessionRef�� �����ϱ� pch.h �� �߰�
	// ���߿� clientSEssion���� �ϴ°� ����
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : Validation üũ
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	// DB���� �÷��̾� ������ �۰Ͽ�
	// Gamesession�� �÷��� ������ ����(�޸𸮿�)
	// ������ ������ ���õ� accountŬ������ ���� �ű⿡ �����ϴ°� �Ϲ���

	// ID �߱�
	static Atomic<uint64> idGenerator = 1;

	{
		// ��Ŷ�� ä��� �۾�, �޸� �󿡵� ����־����
		auto player = loginPkt.add_players();
		player->set_name(u8"DB�����ܾ���̸�1");
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		// proto���� ������� player ��ü�� ��� �־ �Ǵµ� PlayerŬ������ ���������ϱ� �װ� ����
		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		// �޸� ������ ����ֱ� �ؾ���
		gameSession->_players.push_back(playerRef);
	}

	{
		auto player = loginPkt.add_players();
		player->set_name(u8"DB�����ܾ���̸�2");
		player->set_playertype(Protocol::PLAYER_TYPE_MAGE);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.push_back(playerRef);
	}

	// �� ��������� sendBuffer�� ���� ������
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pkt.playerindex();
	// TODO : validation

	// ������ �޸𸮿� ����־�� �ϴ� ����
	// �Ź� DB���� �ܾ���� �ʰ� player������ ���� gameSession�� ���� �״ϱ�
	// �޸𸮿��� ��� ������ �ٷ� ������ �� �ְ�

	// �׷��� ��Ƽ������ ȯ�濡�� ���� �����Ѱ�?
	// ���� �ܰ迡���� ������ ����
	// �÷��̾� �����ϴ� �κ��� Login�κ��̶� �ΰ��ӿ��� �÷��̾ �߰��ϴ� ���� �����ϱ�
	// LOGIN�� �� ó�� ���� ������ �� �ѹ��� �߻��� ���̴�? �� ������ ������ ����
	// ������ �������� ��Ȳ�� �����ϸ� �ȵ�,
	// login��Ŷ�� �����ؼ� ������𼭵� �� �� �ִٰ� �����ؼ� validation�� üũ�ؾ���
	// (�κ񿡼��� login�� �� �ִٴ���)
	PlayerRef player = gameSession->_players[index]; // READ_ONLY?
	//GRoom.Enter(player);
	// job�� ���� ȣ���������
	//GRoom.PushJob(MakeShared<EnterJob>(GRoom, player)); // �ϰ��� ����
	//GRoom.PushJob(&Room::Enter, player);
	//GRoom->PushJob(&Room::Enter, player);
	GRoom->DoAsync(&Room::Enter, player);
	// ==> ������ �� �� ���¶� �ؿ��� pkt�� ����°� �� �̻��� ����
	// ==> ���� �κ��� Enter�� ����� �Ŀ� ����������� �Ű��ְų� �ٲ����
	// �ٽ��� �ƴ϶� �Ѿ��

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

	//GRoom.Broadcast(sendBuffer); // ���������δ�WRITE_LOCK�� ��� ����,
	//GRoom.PushJob(MakeShared<BroadcastJob>(GRoom, sendBuffer));
	//GRoom.PushJob(&Room::Broadcast, sendBuffer);
	//GRoom->PushJob(&Room::Broadcast, sendBuffer);
	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}

//bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt)
//{
//	// TODO : ��¥ ������ �۾�
//	return true;
//}
//
//bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
//{
//	//TODO
//	return true;
//}
