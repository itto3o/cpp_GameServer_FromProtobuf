/*
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

	//WCHAR sendData3[1000] = L"가"; // UTF16 = Unicode (한글/로마 2바이트)

	//while (true)
	//{
	//	Protocol::S_TEST pkt;
	//	pkt.set_id(1000);
	//	pkt.set_hp(100);
	//	pkt.set_attack(10);
	//	{
	//		Protocol::BuffData* data = pkt.add_buffs();
	//		data->set_buffid(100);
	//		data->set_remaintime(1.2f);
	//		data->add_victims(4000);
	//	}
	//	{
	//		Protocol::BuffData* data = pkt.add_buffs();
	//		data->set_buffid(200);
	//		data->set_remaintime(2.5f);
	//		data->add_victims(1000);
	//		data->add_victims(2000);
	//	}
	//
	//	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	//	GSessionManager.Broadcast(sendBuffer);
	//
	//	this_thread::sleep_for(250ms);
	//}

	GThreadManager->Join();
}
*/

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
/* 2023-06-05
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
// PacketHandler.h 자동화 부분에 ㅠ%} 라고 된 곳이 있어서 코드가 안만들어지고 있었다
// 수정 후 출력까지 됨!
*/

// 채팅 실습
/* 2023-06-06
// JobQueue, 진지한 큰 MMO를 만들땐 여기까지는 좀 아쉬움,
// 채팅(그룹방)과 MMO는 종인 한 장 차이,
// 
// ServerCore에서 설정>출력디렉터리에서 Libs\ServerCore 추가
// 
// Proto파일에 message에 string이 들어가게 될텐데, protobuf에서 string은 utf-8만 넣어줄 수 있음
// utf-16이면 일반적인 바이트 배열로 한 뒤 wstring으로 다시 변환해줘야
// 
// 지금 상황에서 string을 그냥넣으면 에러가 나는데,
// protobuf에서 링크에러라는 애가 남
// 특정 버전 protobuf문제라고 함, cmake를 이용해서 protobuf 라이브러리를 만들때
// 원랜 두 옵션을 체크한 후에 만들었는데 모든 걸 다 끈 다음에 다시 만들면 된다고 함
// 나.. 포맷당해서 cmake도 없는데 ㅋㅎ;;
// 그냥 선생님꺼 복붙해야겟다..
// 
// 그리고 이렇게 하면 dll을 요구를 안해서 더이상 필요가 없음, 지워주기
// 
// GameContents 폴더를 만들어서 Room, Player class 만들어주기
// 
// 이제부터도 클라를 쪼개서 서버로 붙이는 연습을 해도 됨
// 
// 지금 만드는 방식의 문제?
// 만약 더미클라가 숫자가 더 늘어나고 게임도 점점 복잡해진다고 하면
// room 안에 몬스터도있고 인공지능, 미사일 원격 투사체 등이 있을텐ㄴ데 걔ㅔ도 실행이 되어야함
// 매번 Broadcast에서 lock을 모두 잡고 있음,
// 경합이 일어나는 부분마다 lock을 잡으면되지않을까 하지만
// 크래시는 나지 않음, 하지만 클라가 보내는 모든 패킷들이 같은 room에서 이뤄진다고 하면
// 모든 room에 있는 요소들에 send작업을 하고 있는데 그 send작업도 굉장히 무거운 작업
// ==> 함수자체가오래걸림, 멀티스레드가 lock을 기다리는 동안 아무것도못함
// ==> jobQueue 활용
//
// C#, C++ 연동이람ㄴ struct player의 패딩 문제 등이 있음 ==> pragma pack() 이용
*/

//JobQueue #1
/* 2023-06-07
// command 패턴에 대해 알아보기(정말 중요!)
// 디자인 패턴 중 하나
// 지금은 식당에서 주문->주문받은 사람(session)이 요리->
// --> 수가 늘어나서 경합이 일어난다면, 계속 기다려야함 주방에서
// ==> 주방장이 따로, 서빙직원 따로(손님이 요청한 사항을 모두 주문서에 적어서 주방장에게 건네줌)
// 서로 영역이 분리, 주문서 만드는 시점, 요리를 하는 시점도 분리
// 주문이 밀린다면 순차적으로 할 것
// 요리가 들어가지 않은 상황이라면 주문도 취소할 수 있음
// ==> command패턴은 요청을 캡슐화해서 주문서로(클래스로, 객체로)만드는 것이 중요
// --> lock을 잡고 경합을 할 필요 없이 순차적으로 담당해서 자기 할 일을 할 수 있음
// 
// Job은 무엇인가? , GameContents 아래에 Job클래스 만들기
// 더미클라는 당장은 필요없어서 GameServer만 시작프로젝트로 설정
// 

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
#include "Job.h"
#include "Room.h"

int main()
{
	// TEST JOB
	{
		// [일감 의뢰 내용] : 1번 유저한테 10만큼 힐을 줘라!
		// 행동 : Heal
		// 인자 : 1번 유저, 10이라는 힐량
		// 바로 요청을 받자마자 실시간 호출이 x
		// 가장 원시적인 방법
		HealJob healJob;
		healJob._target = 1;
		healJob._healValue = 10;
		// 동적할당으로 스마트포인터로 넘겨주는게 좋음

		// 나~중에
		// Execute실행
		healJob.Execute();

		// ==> 일감을 늘릴때마다 클래스를 늘려줘야함
		// template으로 발전시키기 보다는
		// 원시적인 방법으로 채팅 프로그램을 수정해보기
		// 

	}
	// JOB
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

	while (true)
	{
		// 얘(FlushJob)도 잘못해서 여러 스레드들이 동시에 실행하고 있다면 똑같은 문제가 일어날 것
		// 무조건 한명만 담당하게 해야함 (지금은 메인스레드)
		GRoom.FlushJob();
		this_thread::sleep_for(1ms);
	}

	GThreadManager->Join();
}
// 일감을 푸쉬한 순서대로 꺼내쓰는 것이 특징
// 컨텐츠 코드를 만드는게 두 세배가 됨, 함수를 만들때마다 클래스를 만들어야 하는 상황
*/

// JobQueue #2
/* 2023-06-08
// Job.h를 수정해서 공용클래스로 만들어서 관리
// 표준 STL func, lamda를 활용하면 되는데
// 이번시간에서 할 건 아니고, 어떤 원리가 있고 어떤 장단점이 있는지 파악
// Job.h에서 추가로 만들 예정
// HealJob은 노필요
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
#include "Job.h"
#include "Room.h"

// 얘를 job으로 만들어야 함
void HealByValue(int64 target, int32 value)
{
	cout << target << "한테 힐 " << value << "만큼 줌" << endl;
}

class Knight
{
public:
	void HealMe(int32 value)
	{
		cout << "Heal Me!" << value << endl;
	}
};


int main()
{
	{
		FuncJob<void, int64, int32> job(HealByValue, 100, 10);
		job.Execute();
		// job을 만들어줄때 인자들도 같이 기억해서 같이 넘겨줘야
		// 나중에 인자들이 뭐였는지 잘 모를 수 있음
		// ==> 클래스 내부에 Args... args; 멤버변수로 만들진 못하지만 tuple로 가능
		// apply로 하면 호출할땐 인자없이, 대신 생성자에서 인자를 넘겨주기
		// 
		//HealJob healJob;
		//healJob._target = 1;
		//healJob._healValue = 10;

		//healJob.Execute();

	}

	{
		Knight k1;
		// knight의 함수에 funcJob을 호출하고 싶을 때
		// funcJob을 하나 더만들어주기, MemberJob(멤버함수를 호출해주는 job)
		MemberJob job2(&k1, &Knight::HealMe, 10);
		job2.Execute();
	}

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

	while (true)
	{
		GRoom.FlushJob();
		this_thread::sleep_for(1ms);
	}

	GThreadManager->Join();
}

// 매번 job 클래스를만들고 있던걸 날리고
// 일반적으로 코딩하는 것과 마찬가지로 함수단위로 만들어준후,
// 그 함수를 호출할 때 바로 호출하는게 아니라 pushJob을 해서 해당 함수를 간접적으로 호출
// ==> 내부적으로 job으로 만들어져서 jobQueue로 만들어지고, 누군가가 Flush를 해서 실행해줌
// ==> 작업할때 job을 매번 만들ㅇ지 않고도 함수호출하듯 만들 수 있음
// 다음시간엔 람다캡쳐로, 람다캡쳐도 멤버변수로 모두 인자를 들고있기 때문에 다를건없음
*/

// JobQueue#3
/* 2023-06-09
// 최종적으로 작업할 코드는 ServerCore에 새필터를 만들어서 Job class를 만들어주기
// JobQueue는 작성했던 그대로 사용, JobRef는 types.h에 using 달아주기
// gameserver에 있던 job 클래스들은 다 날리기
// 아까운데...... 그래도 깃헙에는 남아있겠지..?
//
// job을 묶어서 인자들일아 같이 들고있는건 통과를 햇는데
// push, pop을 할때 어떻게 할것인가?
// 지금은 일감을 넣어주는 애 따로, flush해주는 애 따로 있음
// 지금은 간단히 room을 하나만 만들어서 mainthread가 체크해주고 있지만
// 나중에가면 room이 어마어마하게 많아짐, 정책에 따라 jobQueue를 룸단위로 배치하는게 아니라
// 객체마다 배치하는 경우가 있음, ==> main스레드가 뺑뺑이 돌면서 flush하는건 좀 그럼
// 스레드끼리 각각 분배해서 일처리가 애매해짐
// ==> 다음시간에 알아보기, 일감 관리

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
#include "Job.h"
#include "Room.h"
#include "Player.h"

//void HealByValue(int64 target, int32 value)
//{
//	cout << target << "한테 힐 " << value << "만큼 줌" << endl;
//}

class Knight : public enable_shared_from_this<Knight>
{
public:
	void HealMe(int32 value)
	{
		cout << "Heal Me!" << value << endl;
	}

	void Test()
	{
		// -hp는 포인터값이 아니고 정수니까 아무런 문젝 ㅏ없다고 지나칠 수 있음
		auto job = [self = shared_from_this()]()//[this]()// [=]()이렇게 하는건 매우위험함, 하나하나 지정하는 습관이 좋음
		{
			self->HealMe(self->_hp); // _hp를 복사하는ㄱ ㅔ아니라 사실 this->_hp가 생략이 되어있기 때문에
			// 위에 [=]이 아닌 [this]를 해주고 있었다, 하지만 Knight가 유효해야 this도 유효하기 때문에
			// 만약 Knight가 소멸이됐다면 this가 오염됨
			// 그리고 Knight가 shared_ptr를 사용하고 있었다고 하면 this포인터를 섞어서 쓰면 안됨
			// refCount에 아무런 도움이 되지않기 때문에
			// ==> shared_ptr를 사용하기ㅗㄹ했으면 enable_shared_from_this를 상속받아서 
			// this가 아니라 shared_from_this()를 호출해서 그걸 self로 복사한 다음에
			// this를 self->로 바꿔줌 ==> refCount +1이 됨
		};
	}
private:
	int32 _hp = 100;
};

//#include <functional>
// ==> std::function을 사용, 온갖 형식의 함수들을 받아줄 ㅅ ㅜ있음
// std::function<void(void)> 는 return void, 인자들은x

//void HelloWorld(int32 a, int32 b)
//{
//	std::cout << "hello world" << std::endl;
//}

int main()
{
	//PlayerRef player = make_shared<Player>();

	//// 이렇게 일반적으로 인수를 호출할때 넣어줄 수 있지만
	//// 함수 포인터로 저장하고 있으면 1, 2 값을 따로 저장할 곳이 없었기 때문에
	//// 함수자를 만들때 tuple에 저장하고 호출할땐 그걸 복원
	//// ==> 람다에서는 아무런 위화감 없이 밑의 코드가 정상적으로  실행이됨
	//HelloWorld(1, 2);

	//
	//							// 람다 캡쳐도 있음 [=] 기능 : functor와 느낌이 비슷함
	////std::function<void()> func = [&player]()//[=]() //[]()로 하면 에러가 남, =은 모든 것을 복사, &은 모든 것을 참조값으로 다 전달
	//							// 그냥 player를 넣으면 player를 지정해서 복사해서 전달, &player도 가능
	//std::function<void()> func = [self = GRoom, &player]() //GRoom을 복사해서 refCount를 유지
	//{
	//	// 우리가 원하는 행동 넣기
	//	//std::cout << "hello world" << std::endl;
	//	HelloWorld(1, 2); // 이런 인자가 익명 함수 내에 같이 전달해서 저장하는 것과 마찬가지로 실행이 다 됨
	//	//GRoom.Enter(player);
	//	self->Enter(player); //shared_ptr을 사용할경우
	//};
	//// 복사해서 안에 넣을때 캡처 모드를 지정해야함

	////TODO

	//// 나중에 func()로 호출해서 실행이 됨
	//func();

	// 보통 이런 걸 closure라고 함
	// 컴파일러가 내부적으로 클래스를 만들어주는데 1, 2 를 넣어준 데이터들도 어딘가에 들고있는 형태로 만들어짐
	// ==> 캡처 모드에 따라서 PlayerRef player; 이렇게 들고있을 것인가 &를 붙여서 들고있을 것인가
	// Job도 std::function 자체를 job으로 여기기만 하면 나머지 문제들이 해결이 됨
	// 
	// C++과 람다의 안맞는 점은 만들자마자 바로 호출하면 상관이 없는데
	// job을 만든 후에 jobQueue에 넣고 누군가 한참후에 그걸 실행
	// ==> 캡처할 당시 넣어줬던 인자들이 유지가 되어야 한참 뒤에 실행할 수 있음
	// 예를들면 캡처해서 &player로 넘겼다고 가정하면, PlayerRef& 형식으로 들고 있따는 이야기고,
	// 이는 shared_ptr을 참조값으로 들고 있는 것이고, 따라서 refCount가 1증가하지 않게 된다
	// ==> refCount가 0이되어 삭제될 수도 있다.
	// 반드시 캡처할 경우, 객체의 생명 주기가 job이 유지가 될때까지는 살아있어야 한다는 보장을 해줘야함
	// 네트워크 코드를 만들 때 세션쪽에서 send, Recv를 걸어줄때 세션이 살아있어야 했떤 것과 마찬가지
	//
	// 주의사항 또 있음
	// Knight클래스 안에 Test90 함수가 있는데 내부에서 잡을 만들어서 전달한다고 하면
	// Knight 안에 _hp가 100, HealMe 함수가 있는데 인자로 value를 받아서 로그를 찍는 함수가 있다고 하면
	// Knight가 실행되는 도중에 job을 만드는 함수에서 HealMe(_hp)를 job에 등록한다ㅗㄱ 하면
	// _hp는 포인터형식이아닌 정수니까 문제가 없을 거라 생각하게 됨
	// 캡처할때 모든 애들을 대상으로 복사하겠다는 굉장히 위험함
	// 하지만 _hp는엄연히 this->_hp기 때문에 [=]를 하면 [this]를 한것과 동일해지고,
	// this는 Knight가 살아있어야 유효해지기 때문에 이 코드 또한 Knight의 생명주기에 따라 위험해짐
	//
	// C++에서 람다랑 shared_ptr를 쓰면 메모리 릭이 발생한다는 글이 많이 올라오는데
	// 사실 그건 class 안에서 callback함수를 쓰면서 shared_ptr을 또 들고 있는 상황이 되는데
	// 그건 사실 사이클이 발생해서 refCount가 0이 되지 않는 상황인거고
	// 람다랑은 상관이 없음
	//

	//{
	//	FuncJob<void, int64, int32> job(HealByValue, 100, 10);
	//	job.Execute();
	//}

	//{
	//	Knight k1;
	//	MemberJob job2(&k1, &Knight::HealMe, 10);
	//	job2.Execute();
	//}

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

	while (true)
	{
		GRoom->FlushJob();
		this_thread::sleep_for(1ms);
	}

	GThreadManager->Join();
}
*/

// JobQueue #4
/* 2023-06-11
// shared_ptr을 들고 있으면 생명주기 관리에는 꽤 좋지만
// 사이클이 일어날 경우 메모리 릭이 발생할 수 있어서 조심해야함
// job안에 sharedptr을 포함하고 있다보니까 이렇게되면 객체가 소멸하지 않을 것
// weak ptr을 쓰거나 ClearJobs같은 함수를 만들어서 jobQueue를 싹 밀어주기
// 
// job을 넣는 애 따로, 실행하는 애 따로 만들어놨음
// 메인 스레드가 루프를돌면서 flush중인데,
// 룸이 한두개면 그렇게해도 되지만, job을 실행하는 애들을 엄청 많이 배치할 수 있음
// 심하면 액터단위로 다 배치
// flush단위가 몇천이 될 수 있는데, 누가 이걸 뺑뺑이를 돌면서 실행해야하는지가 난감,
// 일일이 순회하면서 실행하는 것도 귀찮고, 일감이 없는데도 굳이 매번 루프를 돌면ㅅ ㅓ체크하는중
// --> 조건변수, condition valiable로 일감이 떨어졌을때만 동작할 수도 있겠지만
// 객체가 많아지면 조건변수를 배치하는 것도 힘듦
// push job을 할때 실행까지 담당하게끔 만드는것을 선호
// Session도 Send를 할때 실행까지 할 것인지, 아님 sendQueue에 넣고 빠져나올지 등등
// 처음으로 들어온 애가 아직 false라고 하면 얘가 실행까지 담당하게 햇는데
// 이런 형태로 만들어주면 편리하게 작업할 수 있음
// 
// jobQueue는 다른 곳에서도 jobQueue라고 사용할 수 있으니까 이름수정, lockQueue로 만들어서
// template으로 만들어서 lock을 거는 큐로 공용으로 만들어주기, utils에 넣어주고
// jobSerializer를 jobQueue로 만들기
// 
// 누군가가 따로 챙겨서 실행하는부분이 다 날라감
// 이 방법이 당연하지만 생각할 부분이 많음
// 
// 포트폴리오는 이렇게 만들어도 충분하지만,
// jobQueue방식으로 실행한다면
// 1. 일감이 너무 많이 몰리면? 일감이 0으로 떨어지지 않아 return을 못하고 빠져나갈 수 없음
// 일반적으로 MMO에서는 렉이 걸릴거면 모두에게 렉이 걸려야 공평함
// --> 한쪽만 몰빵해서 실행한다면 아쉬움, (한 애만 렉이 많이걸리게 됨)
// 2. DoAsync를 타고가서 절대 끝나지 않는 상황(일감이 한 스레드한테몰리게 됨)
// execute로 실행을 하면 어떤 이유로 다른 비동기 코드를밀어넣었따고 하면
// 비동기함수 가 따른 객체도 동일한 스레드가 담당해서 여러명을 담당하게 될 수도
// 첫번째 스레드가 여유롭게 시작했지만 다른 애의 jobQueue를 모두 건들여서
// 걔가 모든 경우에서 다 첫번째로 들어오게 되어 모든 비동기 함수를 걔가 실행하게 된다면
// 결국엔 끝이안나고 계속실행, --> 일감 배분 해줘야
//
// Q. clear함수를 추가해서 clear를 추가하기 전에도 pop을 하게 되어 job이 실행되고 나면 
// 동일하게 owner의 refCount가 감소하게되므로 clear가 필요없지 않나요?
// A. 그건 그렇지만, 특정 시점에 오브젝트를 소멸시켜 더이상 job이 실행되지 않기를 원하는 상황도 있을테니까
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
#include "Job.h"
#include "Room.h"
#include "Player.h"

class Knight : public enable_shared_from_this<Knight>
{
public:
	void HealMe(int32 value)
	{
		cout << "Heal Me!" << value << endl;
	}

	void Test()
	{
		auto job = [self = shared_from_this()]()
		{
			self->HealMe(self->_hp); 
		};
	}
private:
	int32 _hp = 100;
};


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

	//while (true)
	//{
	//	GRoom->FlushJob();
	//	this_thread::sleep_for(1ms);
	//}

	GThreadManager->Join();
}
*/

// JobQueue #5
/* 2023-06-12
// JobQueue에 일감을 밀어넣고, 첫번째로 일감을 밀어넣은 애라면 걔가 flush까지 담당
// --> 모든 스레드들이 균등하게 일을 나눠주는 걸 바람
// jobQueue를 호출하고 있는지 아닌지 추적해서, 내가 호출하고 있따고 하면
// 처음으로 들어왔다고 하더라도 다른애한테 떠넘기게
// Job에 GlobalQueue 클래스 만들기
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
#include "Job.h"
#include "Room.h"
#include "Player.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		// countㅅ 설정, 임의의 값을 넣어줄건데 enum으로 빼주기
		// enum의 64와 같은 세부적인 인자들은 하드코딩 했는데,
		// 이런 부분은 그냥 어느정도 체크해서 일감이 제한된 시간에 끝내지 못하는 경우가 많아진다고 하면
		// WORKER_TICK을 좀 늘리는 식으로 자동 보정이 되게끔 만듦
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// 지금까지는 워커 스레드들이 iocp를 두리번거리면서 dispatch를 실행하고 있었음
		// dispatch내부에서는 getQueueCompletionStatus를 하면서 일감이 나오길 기다렸다가
		// 입출력 통지기ㅏ 완료되면 dispatch를 호출,
		// 
		// session의 onRecv를 타고와서 패킷을 조립해서 컨텐츠로 들어가는 상황
		// (패킷이 아니라면 그냥 Session의 ProcessRecv 호출, 패킷세션이라면 PacketSession::OnRecv
		// 그 후에 GameSession.h의 OnRecvPacket이 호출(컨텐츠 단)
		// GPacketHandler를 호출해서 Handle_C_TEST 이런쪽으로 들어가게 됨
		// 그냥 실행해야하는 일감은 GlobalQueue에 넣음
		// 
		// 애초에 시작은 Dispatch, 네트워크 입출력 처리에서 인게임 로직까지 같이 호출하고 있음
		// (패킷 핸들러에 의해)
		// dispatch한 다음에 패킷을 파싱해서 room에 job을 넣어주고 그냥 빠져나오는 식으로 구현하는 것도 있음
		// 네트워크 전달만 담당, 인게임 로직은 따로 만들어서
		// 스레드가 한두개가 아니라 나중에 10개, 20개 단위일텐데
		// 걔네들을 어떻게 배치해서 적절히 분배할 것인지
		// 
		// 지금 현재 상황(네트워크 입출력에서 인게임 로직까지 모두 호출이 되는 상황)에서 문제점은
		// dispatch를 기본값으로 무한대로 넣어놨는데(GetQueueCompletionStatus에서)
		// 네트워크 입출력 함수가 완료되지 않으면ㄴ 빠져나가지 않고 그냥 sleep하게 됨
		// --> dispatch에 타임아웃을 넣게 되면 무한으로 대기하지 않고 지정한 시간만 기다린 후에
		// LastError가 WAIT_TIMEOUT으로 떠서 바로 return false;로 완료됨
		// 
		// 왜 바로 빠져나오게 하느냐?
		// 지금은 간단한 채팅만 했으니까,
		// 패킷을 받아서 처리하는 애 뿐만 아니라 게임 로직을 같이 계속 돌려야하는 애도 있음
		// (몬스터, 화살, 화염구 같은 투사체들이 날라다닐 경우)
		// 클라이언트 쪽에서 패킷을 받아서 처리하는게아니라
		// room의 누군가가 계속 관찰해서 그 로직을 실행시켜야 함
		// --> 모든 애들을 네트워크 단에서 받은 입출력 처리가 호출하게 만들 순 없음
		// 누군가는 room jobQueue자체도 관찰해서 호출해줘야함
		//service->GetIocpCore()->Dispatch();
		service->GetIocpCore()->Dispatch(10);

		// 글로벌 큐 : 네트워크에서 미처 처리하지 못한 애들을 예약하거나 인게임 로직을 예약하거나
		ThreadManager::DoGlobalQueueWork();
		// 만약 얘를 메인스레드가 실행하게 했따고 하면
		// --> 네트워크와 관련된 스레드들은 위에코드에서 실행해주고,
		// 메인스레드가 나머지 전역에 예약된 글로벌큐를 호출해주겠다는 의미
		// 
		// 얘가 지금처럼 함께 넣어주게 되면
		// 스레드를 이용할때 진짜 만능형 직원을 채용하는 느낌(네트워크 입출력에서 인게임 로직까지 같이 호출)
		// 네트워크 통신부하보다 인게임 로직 부하가 더 크다고 하면
		// 인게임 로직에 스레드를 더 많이 배치해서 빠르게 되도록 해야할 것임
		// 반대로 네트워크 처리가 안돼서 dispatch하는 부분이 밀린다고 하면 그것대로 문제
		// --> 모든 스레드들이 만능형으로 변신해서,
		// dispatch(10)으로 10초동안 기다리다가, 바로 빠져나와서 (시간이 남으면) 글로벌 큐의 일을 처리
		// doGlobalQueueWork에서는 tick을 계산해서 일정시간동안만 일을 하다가, 다시 나옴
		// --> 스레드 배치를 어떻게 할 것인가?
		// 어떤 곳에선 모든애들을 iocpCore를 이용하는 경우도 있음
		// 우리가 원하면 일반 queue를 사용하듯 강제로 완료통지가 뜨게끔 넣어줄 수도 있음
		// 일반 컨텐츠 코드도 iocpCore에 넣을 수 있음
		// --> 모든 일꾼들에게 일감이 균등하게 배분
	}
}

class Knight : public enable_shared_from_this<Knight>
{
public:
	void HealMe(int32 value)
	{
		cout << "Heal Me!" << value << endl;
	}

	void Test()
	{
		auto job = [self = shared_from_this()]()
		{
			self->HealMe(self->_hp);
		};
	}
private:
	int32 _hp = 100;
};


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
		GThreadManager->Launch([&service]()//[=]()
			{
				while (true)
				{
					//service->GetIocpCore()->Dispatch();
					DoWorkerJob(service);
				}
			});
	}

	// 처음엔 무한루프를 돌면서 DoWorkJob같으걸 해줬는데,
	// 그렇게 하면 일이 균등하게 분배되지 않을 것임
	// ==> 일단 위에 함수를 만들기
	
	// mainthread는 딱히 할일이 없으니까 얘도 만능형 일꾼이 되도록
	DoWorkerJob(service);

	// 일감이 균등하게 배분은 됐으나 일감이 너무 몰리면 하나의 스레드에 과부하가 온다는 건 변하지 않음,
	// --> 어느정도 호출을 하다가 도저히 답이 없다고 하면 적당히 빠져나오도록 수정
	// JobQueue.cpp에서 무한 루프를 돌면서 남은 일감이 0개면 종료하는 부분에
	// 하나를 더 체크해서 현재 시간을 체크한 다음
	// endTickCount보다 더 크다고 하면 원래 할당받은 시간보다 더 소모를 한 셈이니까
	// globalQueue에 넣고 빠져나오기

	GThreadManager->Join();
}

// 어 나 빌드하는 과정에서 *.pp.h를 못찾는다는 로그를 봤는데...?
// protocol.proto를 바꿔서 빌드해보니까 코드가 자동으로 만들어지는 부분은 처리가 되고 있긴한데
// 그럼 그냥 이미 .h파일이 없어져서(이전에 계속 했었으니까) 그렇게 뜨는 건가

// Q. 스레드들이 DoGlobalQueueWork함수를 이용해서 jobQueue를 처리하고 있는데
// 이렇게 되면 job을 처리하는 도중에 데이터가 겹쳐서 race condition상황이 발생하는지?
// A. GGlobalQueue에 들어갔다가 다시 여러 스레드에 배분되어 실행된다면 물론 멀티스레드 이슈가 생길 수도 있음
// 다만 대부분의 경우 여러 jobQueue 간에 공유하는 데이터는 생각보다 많이 생기지 않음
// Zone단위로 JobQueue를 배치하는 경우 jobQueue 간 데이터 공유가 거의 없고(하나의 지역에서 모든 일들이 일어나기 때문)
// Actor 단위로 jobQUeue를 배치하는 경우 필연적으로 아주 잠시 Lock을 걸어서 데이터를 빼오거나
// 아니면 락을 걸지 않아도 크래시가 나지 않는(ex. 배열)을 사용해서 우회
//	--> 부가설명,
// 락을 걸지 않으면 레이스컨디션은 발생하지만, 크게 상관없는 경우도 있음
// 액터단위로 배치한다 가정할때 실제로 자신의 정보를 건드리는 것은 무조건 해당 액터의 jobQueue에서 일어나기 때문에 문제 없음
// 하지만 외부의 다른 액터가 해당 정보를 긁어올 필요가 종종 생김
// Ex. 유저 2명이 서로 전투가 일어났고, 유저 단위로 jobQueue가 걸려잇는 경우,
// 상대방의 hp가 얼마인지 얻어오고 싶은데, 그렇다고 상대 객체에 lock을 걸고 getHp를 하기엔 아쉬우니
// 락을 걸지 않고 일단 갖고 옴
// --> 상대측에서 setHp가 병렬로 실행돈다면 hp가 정말 '현재 최종값'이 맞으리라는 보장은 없지만,
// 그정도로 정밀하게 타이밍을 맞춰야 할 필요는 없으니 넘어가는 것
*/

// JobTimer
/* 2023-06-13 */
// 지난 시간에 했던 것 수정
// : shared_ptr를 사용해서 하고 있는데, refCount이 되기 때문에 오염된 메모리를 건드리거나 하는 확률은 줄이지만
// 메모리 릭이 날 상황이 큼
// --> 서버를 오랫동안 띄우면 한 2주동안 연속해서 띄우면 한번씩은 내리고 다시 띄웠어야
// --> GameSession.h에 소멸자 로그를 해놧었는데 언제부턴가 소멸자 로그가 찍히지 않고 있었다..!
// GameSession에 playerREf를 만들어놨는데, Player.h에서는 GameSessionRef를 들고 있음
// player도 room안에 넣어놨는데 빠져나오지 못해서 거기서도 사이클
// --> login을 할 때 players에 연결을 해줬었음
// player가 선택해서 C_ENTER를 호출하면 어떤 플레이어로 로그인하고 있는지, 어떤 방에 있는지
// gamesession에서 들고 있다가 나갈때 정리해주는 그런게 필요함
// ==> GameSession.h에서 PlayerRef로 currentPlayer 추적
// 
// breakPoint를 잡지 않고, 더미 클라에서 연결했다 끊었다를 반복해보고 세션을 늘렸다 줄였다 해보면
// 메모리가 우상향으로 쭉 늘어나면 메모리 릭일 것임
// 
// 오늘은 JobQueue.hㅔㅇ 일감을 밀어넣는 작업까진 했는데
// 이것만 이용해서 게임만들려면 아쉬움
// 스킬, AI이건 무한적으로 tick을 돌면서 막 실행하는 경우는 없음
// 클라는 rendering을 해야하니까 그래픽을 계산해서 그려야하고 그걸 빨리 그릴수록 화면이 부드러워 지니까
// 온갖성능을 다 끌어올려서 loof를 돌면서 처리하겠지만
// 서버코어는 그렇게 1초에 몇백번씩 한 오브젝트를 갱신할 필요가 없음,
// 보통은 1초단위, 인공지능은 2초, 1초 정도로 주기적으로 체크하면서
// 객체를 업데이트하면서 로직을 실행(길찾기, 앞으로 이동, 데미지 판별 같은 것들)
// 빠르게 연산할필요는 없음(클라에 비해)
// 
// 클라쪽에서는 별로 신경을 안쓴다고 하면 쿨타임 체크같은건 현재시간을 가져온다음에
// now에 +1000을 더한게 1초 후의 시간이니까 그게 end라고 하면
// 현재시간이 end를 지났는지 안지났는지 체크하기 위해 loof를 돌면서 매프레임마다 체크하다가
// 시간을 다시 계산해서 더 큰지 아닌지를 판별, 더 크면 1초가 경과되었다고 판별,
// 이런식으로 loof를 돌면서 처리하는게 일반적
// 서버같은 경우(MMO같은 경우는 특히,) 몇백만번의 loof를 도는건 말도안됨
// --> 예약 시스템이 필요
// 1초 후ㅇ에 실행해줘 하는걸 걸어주면 정말 1초후에 실행되는 그런거
// Unity에서도 코루틴을 이용해서 buildyoursecond? 같은걸 이용해서 쿨타임을 만들곤 함
// 
// 지금까지는 일감을 즉흥적으로 만들어서 꽂아주는식으로만 해놨고,
// 처음으로 들어온 애라면 실행시키게 해놨는데
// 예약을 해야한다고 하면, 당장은 걔를 실행할필요는 없고 1초후에 일감에 등록을해야한다는거니까
// 지금 하는 방식과는로직이 달라져야 함
// 
// 오늘은 중앙통제를 이용한 방법
// jobQueue를 이용해서 job을 들고있다고 가정, 우선순위 큐같은 예약한 잡을 들고있따고 하면
// 언제 실행해야할지 당장은 예측하기 힘듦, 매번 체크해서 실행하려면 애매함
// --> jobQUeue마다 하나씩 만들어서 거기에 들고있기 보다는,
// 중앙 시스템을 만들어서 중앙에서 예약된 일감들을 들고 있다가, 빠르게 체크해서 뿌려주는 방식
// --> Job에 JobTimer 클래스 추가
// 
// 여러 일들이 예약된 후에 누군가 하나의 스레드가 담당하게하면 불공평할 수도 있음
//  모두가 다 만능형 직원이니까
// -> ThreadManager.h에 새로운 함수 추가
// 
// 지금 JobQUeue.h에 만든 DoTimer 같은 부분이 경합이 너무 심하다고 생각되면
// jobTimer를 수정해서 더 효율적으로 만드는 것도 고려
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
#include "Job.h"
#include "Room.h"
#include "Player.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		service->GetIocpCore()->Dispatch(10);

		// 예약된 일감 배분 코드 
		ThreadManager::DistributeReservedJobs();

		ThreadManager::DoGlobalQueueWork();
	}
}

class Knight : public enable_shared_from_this<Knight>
{
public:
	void HealMe(int32 value)
	{
		cout << "Heal Me!" << value << endl;
	}

	void Test()
	{
		auto job = [self = shared_from_this()]()
		{
			self->HealMe(self->_hp);
		};
	}
private:
	int32 _hp = 100;
};


int main()
{
	// 예약이 필요한 경우(사용할 때)
	GRoom->DoTimer(1000, [] {cout << "Hello 1000" << endl; }); //1000이 1초
	GRoom->DoTimer(2000, [] {cout << "Hello 2000" << endl; });
	GRoom->DoTimer(3000, [] {cout << "Hello 3000" << endl; });

	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()//[=]()
			{
				while (true)
				{
					DoWorkerJob(service);
				}
			});
	}

	DoWorkerJob(service);

	GThreadManager->Join();
}

// MMO기준 C++은 게임엔진 사용없이 자체제작한대
// FPS장르는 dedicated서버라고 언리얼 내장 서버를 이용하긴 함
// server라는 오브젝트를 만들어서 script를 통해 서버를 띄우고 서버빌드로 빌드해서 사용하나요?
//  --> 유니티를 켜서 서버를 구동하는게 아니라 별도의 프로그램으로 서버를 구동