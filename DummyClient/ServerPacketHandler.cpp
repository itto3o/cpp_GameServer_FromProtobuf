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
		// ĳ���� ����â(��ĳ�� �ϳ������ٸ� ĳ���͸� ����� �κ�)
	}

	// ���� UI ��ư ������ ���� ����
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0); // ������ ù��° ĳ���ͷ� ����

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);
	// �� �� �ܰ踦 ��ġ�� ������ ���߿� ��ε�ĳ��Ʈ�� �� ���� ����ؼ�

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	// TODO : room, player ������ ���� ��Ī������ ������ִ°� ����
	// �ش� ���� �÷��̾� ����� �����ͼ� �������ϴ� ��
	// ������ ä���ϴ°� �����̴ϱ� ä�ø޽�����
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
//	cout << pkt.id() << " " << pkt.hp() << " " << pkt.attack() << endl; //<<���� ���������ʴµ�?
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

