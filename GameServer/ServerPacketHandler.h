#pragma once
//#include "BufferReader.h"
//#include "BufferWriter.h"
#include "Protocol.pb.h"

// bool�� �����ϰ� PacketSessionREf�� ���ڷ�,(�������� ���� ������ ���۰� 1 �����ϴ� ����� ���̱� ����
// , ��������� �븮�� ����)
using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
// uint64������ŭ��ŭ�� �迭�� ���� �ٷ� ȣ�����ֵ�����
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

// TODO : �ڵ�ȭ
enum : uint16
{
	PKT_S_TEST = 1,
	PKT_S_LOGIN = 2,
};

// Custom Handlers
// ��׵��� �����θ� �ڵ�ȭ���� �ʰ� ������ ������ �۾��ڰ� ���� ��������
// ���������� ������ ������ �������� �� �� �����ϱ� �ڵ�ȭx, �˾Ƽ�
// ������ �����ϴ� �κб����� �ڵ�ȭ TODO
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt); // ����ϴ� �ʿ��� ���ϰ� �۾��� �� �ְ� pkt�� ������

class ServerPacketHandler
{
public:

	//TODO : �ڵ�ȭ
	static void Init()
	{
		// GPacketHandler[]�� ����� � ���� �Ұ���
		// �ϴ� invalid�� ��� �ʱ�ȭ, �߸��� ��Ŷ���� ���� invalid�� ȣ��� �� �ְ�
		for(int32 i  = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

		// �Լ��� byte, int32�� ���Դٸ� �����Լ��� ��Ŷ�� ������� �� ȣ���Ű��
		// ���� handlePacket�� ȣ���Ұǵ�, packettype�� S_TEST, func�� Handle_S_TEST��
		// �� �κ��� �ٵ� Ŭ�󿡼� ����� �Ǿ�� ��
		// Init�� ������ �׳� GameServer���� �ϵ���
		GPacketHandler[PKT_S_TEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_TEST>(Handle_S_TEST, session, buffer, len); };
	}
							// ���� ������ ���´��� Ȯ���ϱ� ����
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}

	// TODO : �ڵ�ȭ, ��Ŷ ������ ���� �þ(S_LOGIN ���)
	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt) { return MakeSendBuffer(pkt, PKT_S_TEST); }

private:
	// �޴� ���� �Լ�
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		// ��Ŷ�� ������ְ�, ParseFromArray���, (Ŭ�󿡼� �޾Ҵ� �κ��� �ڵ�ȭ���ֱ� ����)
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};

// ������ private�� static���� �ű��
//template<typename T>
//SendBufferRef _MakeSendBuffer(T& pkt, uint16 pktId)
//{
//	const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
//	const uint16 packetSize = dataSize + sizeof(PacketHeader);
//
//	SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
//	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
//	header->size = packetSize;
//	header->id = pktId;
//	ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
//	sendBuffer->Close(packetSize);
//
//	return sendBuffer;
//}