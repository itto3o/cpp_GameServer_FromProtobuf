#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"

int main()
{
	ClientPacketHandler::Init();

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

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}

// 패킷 자동화 #1
/* 2023-06-04
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
// 오 됐다 email때문이었구나 휴
*/

// 패킷 자동화 #2
/* 2023-06-05 */
// 지난번에 만들었던 코드를 자동화 시키기
// 자동화된 코드로 파일이 자동으로 만들어졌으면 좋겠다.!
// 
// 자동화해야할 부분?
// enum 안쪽, Handle_S_TEST 선언, Init안의 아래 부분, 버전으로 오버로딩하는 작업
// 서버끼리 주고받는 패킷도 ServerPacketHandler라고 하면 헷갈릴 여지가 있으니까
// 누가 나한테 보냈는지를 기준으로 짓기, 클라이언트에서 나한테 보낸다면 그냥 클라핸들러로
// ==> 이름을 뒤집기, 나중에 DBPacketHandler 이런식으로 가능하니까
// 
// 툴은 C#이나 C++로 만들경우도 있는데, 회사에선 가장 많이 python을 사용
// 1. 추가>새솔루션폴더>Tools, Server폴더 안에 Tools 폴더 만들기
// 
// 파이썬 설치 필요, visual studio installer 실행, python 개발 선택 후 수정,
// tools>추가>새프로젝트>python 어플리케이션
// 
// python 환경변수 추가,
// jinja, pyinstaller 다운받기
// python3.9 우클릭>패키지관리> jinja2, pyinstaller 설치(exe파일을 만들때 도움을 주는 애들)
// 
// Packethandler까지 만들었으면 얘를 매번 실행하는 것도 귀찮으니까
// exe파일로 만들기, pyinstaller 활용,
// PacketGenerator 폴더에서 MakeExe.bat 파일을 만들기
// --onefile (하나의 파일로 묶어달라) , PacketGanerator.py파일과 ProtoParser.py가 두개라
// bat 파일을 실행하면 dist안에 exe파일이 생김,
// window depender가 악성코드로 인식할 때는 삭제하지 않도록 설정
// build, templates 폴더들을 삭제하기 위해 bat파일에 추가
// GenPackets.exe를 실행하면 testPacketHandler.h 가 만들어짐
// 
// PacketGenerator.py 실행이 안돼서 왜 안되나 했는데 packetHandler.h에서 주석에 쓴 {{조차 인식해서
// 오류라고 뜨는거였네..
// 
// 
// 매번 눌러주기는 또 귀찮으니까
// 외부에서 관리하던걸 옮겨주기
// MakeExe.bat을 만들어서 실행,
// 
// GenPackets.exe와 templates폴더를 복사해서 Server의 Common>Protobuf>bin 안에 붙여넣기
// GenPackets.bat도 수정
// GenPackets.exe --path 인자 넣어주고, --output, --recv 다 인자로 넣어주기
// 똑같은 방법으로 Server도 만드는데 recv, send 뒤집어주기
// 
// GetPackets.bat을 실행한 후에 Templates/PacketHandler.h 도 날라가고 있어서
// DEL 옆의 /S를 모두 삭제하기(S는 재귀적으로 안에까지 들어가라는 의미)
// 
// 추가적으로 Protocol.proto에 예를들어, C_MOVE를 설계한다고하면
// 저장한 후 gameserver를 빌드하면 자동으로 만들어질텐데,
// 이때 뜨는 오류는 새로 만들어진 함수 구현이 없다는 거니까
// Handle_C_MOVE 함수를 만들어서 콘텐츠 구현을 하면 됨
// (난 왜 안되냐)
// 1. protocol에 C_move 멤버가 없다는 오류가 뜨고
// 2. PKT_S_TEST 재정의 : 이전 정의는 열거자 입니다 오류가 떠
// 
// 아무튼 됐다고 하면 더미클라의 OnConnected에서 C_MOVE 패킷을 만들고 sendBuffer로 보낼 수 있음
// 
// 시작을 dummy, gameserver둘다 킨다고 하고 실행하면 크래시가 날텐데
// 1. GameSession.cpp의 session = GetPacketSessionRef()인데 PacketSessionRef()여서 null crash가 남
// 2. ServerPacketHandler.h의 Init을 어딘가에서 한번 호출해야 할텐데 그걸 DummyClient에서도 해줘야함
// 
// 나왜근데 더미클라는 빌드 되는데 서버는 빌드가 안되지
// 진짜 뭐지..? 심지어 serverPackethandler랑 ClientPacketHandler랑 선생님꺼랑도 똑같은데 왜안되지..?
// 뭐지 더미클라 pb.h랑 pb.cc는 왜 또 난리지
// 이번엔 또 재정의 오류만 뜨네 진짜뭐지
// 
// 진짜 뭔데.................?
// 
// 아니 ClientPacket.h에서 오류나길래 거기가문젠줄 알았는데
// .cpp에 누가(???) ServerPackehtHandler.h를 추가해놔서 ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ
// 아마.. 뭐어케고치다가 그렇게 된 거 같은데
// 진짜어이없네 그게 문제엿다니;;
// 이미 브랜치도 다파고 쌤코드도 다 다운받았는데;;; 진자 뭔 ㅋㅋ
// 
// 나근데 왜 애 로그가 안찍히냐
// 아나 이건 또 무슨 경우임
// ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ키ㅏ머ㅣㄹ아먼이ㅏㅁ나ㅣ멈나ㅣㄹㄴㅁ 
// .....난.. 모르겠다..... 몰라.. 다음에 고칠까....
// 