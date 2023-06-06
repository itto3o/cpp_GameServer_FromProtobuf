#include "pch.h"
#include "ServerPacketHandler.h"
//#include "BufferReader.h"
//#include "Protocol.pb.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
		return false;

	if (pkt.players().size() == 0)
	{
		// 캐릭터 생성창(부캐가 하나도없다면 캐릭터를 만드는 부분)
	}

	// 입장 UI 버튼 눌러서 게임 입장
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0); // 지금은 첫번째 캐릭터로 입장

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);
	// 이 두 단계를 거치는 이유는 나중에 브로드캐스트를 할 때를 대비해서

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	// TODO : room, player 정보를 완전 대칭적으로 만들어주는게 좋음
	// 해당 방의 플레이어 목록을 가져와서 렌더링하는 식
	// 지금은 채팅하는게 목적이니까 채팅메시지만
	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;
	return true;
}

//void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
//{
//	BufferReader br(buffer, len);
//
//	PacketHeader header;
//	br >> header;
//
//	switch (header.id)
//	{
//	case S_TEST:
//		Handle_S_TEST(buffer, len);
//		break;
//	}
//}

//bool Handle_S_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt)
//{
//	//Protocol::S_TEST pkt;
//
//	//ASSERT_CRASH(pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)));
//
//	cout << pkt.id() << " " << pkt.hp() << " " << pkt.attack() << endl; //<<여기 들어오지도않는데?
//
//	cout << "BUFSIZE : " << pkt.buffs_size() << endl;
//
//	for (auto& buf : pkt.buffs())
//	{
//		cout << "BUFINFO : " << buf.buffid() << " " << buf.remaintime() << endl;
//		cout << "VICTIMS : " << buf.victims_size() << endl;
//		for (auto& vic : buf.victims())
//		{
//			cout << vic << " ";
//		}
//
//		cout << endl;
//	}
//
//	return true;
//}
//
//bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
//{
//	return true;
//}

