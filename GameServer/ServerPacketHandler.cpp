#include "pch.h"
#include "ServerPacketHandler.h"
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

bool Handle_S_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt)
{
	// TODO : ��¥ ������ �۾�
	return true;
}
