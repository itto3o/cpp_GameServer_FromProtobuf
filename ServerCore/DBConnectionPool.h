#pragma once
#include "DBConnection.h"

/*----------------------
	DBConnectionPool
------------------------*/
// 전역으로 딱 하나만 들고 있을 것
// 어떤 스레드건 ㅜ가 필요할지 예측할 수 없어서 전역으로 놓고 필요한 애가 꺼내씀
// ==> CoreGlobal.h에 넣어주기
// 만약 DB를 사용안하는 경우엔 컨텐츠 단에서 전역변수로 선언하는게 나음
class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	// connection을 만들어주는 애
	// connection을 만들 개수, 커넥션 만들때 조건(어떤 디비, 어떤 환경, 어떤 조건 등등)
	bool				Connect(int32 connectionCount, const WCHAR* connectionString);
	void				Clear();

	// 얘를 shared_ptr로 해줄 필요는 없는게 단순하게 pop한 후에 다시 바로 반납할 예정
	DBConnection*		Pop();
	void				Push(DBConnection* connection);

private:
	USE_LOCK;
	SQLHENV					_environment = SQL_NULL_HANDLE; //Handle ENV, 환경 담당하는 Handle
	Vector<DBConnection*>	_connections;
};

