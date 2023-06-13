#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override
	{
		//Protocol::C_MOVE movePacket;
		//auto sendBuffer = ServerPacketHandler::MakeSendBuffer(movePacket);
		//cout << "Connected To Server" << endl;

		Protocol::C_LOGIN pkt;
		// 인증 서버는 외부에서 많이 설계함
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		ServerPacketHandler::HandlePacket(session, buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}
};

int main()
{
	ServerPacketHandler::Init();

	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>, // TODO : SessionManager 등
		1);
		//100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	Protocol::C_CHAT chatPkt;
	chatPkt.set_msg(u8"Hello World !");
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);

	while (true)
	{
		service->Broadcast(sendBuffer);
		// 브로드 캐스트 패킷 넣어주기
		// Service에서 보면 지금까지 만들어준 session들을 들고있음
		// 서버 코어에서 refcount관리용으로 만들긴 했는데 이걸 재사용해서
		// 편하게 브로드캐스트할 ㅅ ㅜ있게끔 Service.h에 함수 만들기
		// sendbuffer를 받아서 현재 만들어준(service에 들고있는) 모든 세션에 뿌려주는 작업
		// 아 여기는 Server가 아니라 현재까진 Player, Room class가 없구나
		// 그래서 Service에 만들어주신 것 같기도 하고
		// server에서는 CHAT pkt을 받으면 그걸 다시 Room안의 broadcast호출
		this_thread::sleep_for(1s);
	}
	GThreadManager->Join();
}