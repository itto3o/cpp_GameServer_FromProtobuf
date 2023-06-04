#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"

int main()
{
	ServerPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}				
			});
	}	

	WCHAR sendData3[1000] = L"가"; // UTF16 = Unicode (한글/로마 2바이트)

	while (true)
	{
		Protocol::S_TEST pkt;
		pkt.set_id(1000);
		pkt.set_hp(100);
		pkt.set_attack(10);
		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(100);
			data->set_remaintime(1.2f);
			data->add_victims(4000);
		}
		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(200);
			data->set_remaintime(2.5f);
			data->add_victims(1000);
			data->add_victims(2000);
		}

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}

// 패킷 자동화 #1
/* 2023-06-04 */
// 콘텐츠를 만들자니 아직까진 반복적이고 귀찮은 작업이 많음,
// --> 컨텐츠 단에서는 편하고 실수없이 할 수 있도록 작업
// 
// 무엇을 자동화할것인가?
// 1. 프로토버프파일을 만들고 배치파일을 실행하는 것도 귀찮고
// 실행된걸 프로젝트에 옮겨오는것도 귀찮음
// ServerPackethandler.h에 보면 S_TEST의 아이디를 부여하고 1대1로 만들어서 직접 연결해줌
// 패킷해더를 참고해서 switch case에따라 id에따라 구분하는중
// 100개가 넘어가면 작업하기 힘듦..
// 
// 구조정리부터,
// Server/protoc-3.17.1-win64를 어디서관리할지, Common 폴더를 만들어서 거기에서 라이브러리 다 관리
// 
// proto 파일을 다시 보면, S_TEST는 패킷전송을 위해 만들었지만,
// BuffData는 어찌보면 구조체로 활용할 수 있음
// 한 프로토파일에 관리하는건 바람직하지 않음,
// 최대한 용도에 따라 구분해서 만들기
// 너무 proto파일이 커지면 수정할 때 .cc, .h파일이 모두 바뀌게 되면 빌드할때도 힘듦
// ==> proto파일을 두개 만들기
// proto파일 세개를 GameServer에 넣기
// 
// enum.proto에는 enum을 넣기,
// 규칙상 proto에는 0이 꼭 있어야 해서 아무런 타입도 없는 애를 0으로 지정하는걸 추천
// 
// struct.proto에는 전송하는 패킷이아닌,
// 일반적인 struct용도로 활용할 데이터들
// 
// 프로토파일끼리도 서로 참조할 수도있음
// import "Enum.proto"를 하면 가능
// 
// bat파일도 Protocol 폴더에 넣기
// bat파일을 실행해서 생겨난 .h, .cc파일들을 모두 gameserver에 복붙을 한 다음에
// protocol 필터에 추가
// 
// ==> 이 작업을 매번 반복하기 쉽지않으니까
// bat파일에서 자동으로 하게 해주기
// XCOPY는 다른곳으로 복사, /Y는 덮어쓰기
// 
// proto.exe를 못찾을 수도 있으니까 미리 pushd %~dp0로 배치파일이 실행될 장소를 미리 정함,
// 경로를 수정
// 
// .cc파일에 속성을 가서 미리컴파일된 컴파일러 사용안함으로 바꿔줘야
// 
// proto파일을 고치면 배치파일을 매번 실행해야하는데 그것도 너무귀찮음
// ==> 배치파일을 알아서 하기(빌드할때 이벤트를 걸어줄 수 있음)
// 프로젝트 속성>빌드이벤트 에서 bat파일을 call하면 됨
// 빌드이벤트 단점은 소스코드가 아무런 변화가없으면 빌드이벤트가 실행되지 않음,
// proto파일들은 추적대상?이 아니다보니까 참조로 넣어줬으니까.(문서 편집기 용도로활용하고 있으니까)
// 추가, 삭제해도 인지를 못하니까
// ==> visual studio를 끈 상태에서 프로젝트 파일을 열어(GameServer.vcxproj)
// 밑의 ItempGroup에서   <ItemGroup> UpToDateCheckInput을 추가하면 됨
// 
// ==> 이제 SeverPacketHandler.h 에서의 S_TEST, 사실상 그렇게 좋은 방법이 없음
// 패킷을 설계할때(proto파일에) 번호를 지정할 수 있으면 좋을텐데
// proto3문법에서는 할 수 없음,
// 안에서 enum값을 넣어주고 Packet_Id = 1로 정의하는게 가능,
// enum에서는 0으로 시작을 해야하니가 None = 0;이 추가되어야
// 다른 message가 만들어졌을 때도 똑같이 enum PacketId id = 2; 이런식으로 해야함
// enum값 자체가 공용이 아니라, 메시지 안에 들어가있어서
// 동일한 애로 인식하지 않고 s_TEST안의 PacketId로 인식
// ==> Protocol::S_TEST::Packet_Id 로 꺼내 쓸 수 있게 됨
// ==> 마음에 안들긴 하지만 실수해서 겹친다고 가정하면, 코드가 꼬일 확률이 있음
// 고정해서 만들게 되면 해킹을 방비할때 packet조작에 취약함 대부분 실시간 테스트
// 서버에 전송되는걸 캡처해서 살펴보다가 분석, ==> 패킷 아이디를 주기적으로 섞어주는게 중요
// 결국엔 자동화 툴이 필요함
// 
// --> ServerPacketHandler를 수정, 얘를 기반으로 만들거라 깔끔하게 작업하는게 중요
// 
// 엥 나 근데 왜 커밋했는데 contribution에 안뜸?
// email이 다른 것 같아서 같은 이메일로 보내보겠다..
//