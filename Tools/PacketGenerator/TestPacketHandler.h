#pragma once
//#include "BufferReader.h"
//#include "BufferWriter.h"
#include "Protocol.pb.h"

// bool을 리턴하고 PacketSessionREf를 인자로,(참조값을 받은 이유는 레퍼가 1 증가하는 비용을 줄이기 위해
// , 성능향상을 노리기 위함)
using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
// uint64개수만큼만큼의 배열을 만들어서 바로 호출해주도록함
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];


// 모든 패킷 목록을 만들어줌
enum : uint16
{
	// 파이썬 코드인지 일반 샘플 코드인지 구분할 수 없으니까 jinja2만으 ㅣ문법을 사용
	// 내부에 있는 파이썬 코드가 실행됨, 
	// 안에 있는 내용 구성, 패킷이 모두 PKT로 시작하니까 PKT_로 pkt.name으로 치환해줌
	// ==> for문을 돌면서 밑의 PKT_S_TEST = 1, 처럼 코드가 작성됨
	// 에 -를 하는 이유는 PKT앞에 한칸을 띄웠느데 얘를 그대로 적용할지 아님 붙여서 만들어줄지(비주얼 적으로)
	PKT_C_TEST = 1000,
	PKT_C_MOVE = 1001,
	PKT_S_TEST = 1002,
	PKT_S_LOGIN = 1003,
// TODO : 자동화
//PKT_S_TEST = 1,
//PKT_S_LOGIN = 2,
};

// Custom Handlers
// 얘네들은 구현부를 자동화하지 않고 구현은 컨텐츠 작업자가 직접 만들어야함
// 컨텐츠에서 무엇을 할지는 툴에서는 알 수 없으니까 자동화x, 알아서
// 하지만 선언하는 부분까지는 자동화 TODO
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);

// 받는 쪽의 패킷만 하면 됨(클라 입장에서 서버에서 보내준패킷만 핸들링)
bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
//bool Handle_S_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt); // 사용하는 쪽에서 편하게 작업할 수 있게 pkt를 보내기

//class ServerPacketHandler
// output을 따로 받아서 설정
class TestPacketHandler
{
public:

	//TODO : 자동화
	static void Init()
	{
		// GPacketHandler[]의 몇번이 어떤 일을 할건지
		// 일단 invalid로 모두 초기화, 잘못된 패킷으로 오면 invalid가 호출될 수 있게
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

		// 자동화

		// 함수에 byte, int32가 들어왔다면 람다함수로 패킷을 만들어준 후 호출시키기
		// 밑의 handlePacket을 호출할건데, packettype을 S_TEST, func을 Handle_S_TEST로
		// 이 부분은 근데 클라에서 등록이 되어야 함
		// Init은 지금은 그냥 GameServer에서 하도록
		//GPacketHandler[PKT_S_TEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {return HandlePacket<Protocol::S_TEST>(Handle_S_TEST, session, buffer, len); };
	}
	// 누가 나한테 보냈는지 확인하기 위함
	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}

	// send 패킷에 대해 자동화
	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt) { return MakeSendBuffer(pkt, PKT_S_TEST); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	// TODO : 자동화, 패킷 개수에 따라서 늘어남(S_LOGIN 등등)
	//static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt) { return MakeSendBuffer(pkt, PKT_S_TEST); }

private:
	// 받는 역할 함수
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		// 패킷을 만들어주고, ParseFromArray사용, (클라에서 받았던 부분을 자동화해주기 위함)
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

// 위에서 private의 static으로 옮기기
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