#include "pch.h"
#include "ClientPacketHandler.h"
#include "..\DummyClient\ServerPacketHandler.h"
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

bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt)
{
	// TODO : 진짜 컨텐츠 작업
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	//TODO
	return true;
}
