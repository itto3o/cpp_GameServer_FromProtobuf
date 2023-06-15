#include "pch.h"
#include "DBConnectionPool.h"

/*----------------------
	DBConnectionPool
------------------------*/
DBConnectionPool::DBConnectionPool()
{
}

DBConnectionPool::~DBConnectionPool()
{
	Clear();
}

bool DBConnectionPool::Connect(int32 connectionCount, const WCHAR* connectionString)
{
	// 연결 맺어주기, 처음 서버가 뜰때 딱 한번만
	WRITE_LOCK;

	// 어떠한 사유로 사용하는지 계속 적어주게됨
	// 처음에는 ENV를 연동해줄 것이다. 두번째는 NULL, 세번째는 environment자체
	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment) != SQL_SUCCESS)
		return false;

	// handle을 이용해 ODBC 버전을 지정해줄 수 있음, 3점대 버전
	if (::SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		return false;

	// conncection count 개수만큼 연결을 맺어주기
	// DBConnection 객체로 요청을 보낼 것임
	// 한 스레드만 요청을 한다고 보장은x,
	// 이럴 경우 connectioncount를 스레드 개수만큼 만들어서 하나씩 돌려사용한다고 보면 됨
	for (int32 i = 0; i < connectionCount; i++)
	{
		DBConnection* connection = xnew<DBConnection>();

		// 환경을 통해서 실제적으로 conncection을 만들어줘야함
		if (connection->Connect(_environment, connectionString) == false)
			return false;

		_connections.push_back(connection);
	}

	return true;
}

void DBConnectionPool::Clear()
{
	WRITE_LOCK;
	
	// 정리 작업
	if (_environment != SQL_NULL_HANDLE) // null이 아니라면 free
	{
		// 내가 어떤 애를 닫아주고 있는지 handle type을 같이 적어주고 있음
		::SQLFreeHandle(SQL_HANDLE_ENV, _environment);
		_environment = SQL_NULL_HANDLE;
	}

	// connection도 하나씩 다 메모리를 날려주기
	for (DBConnection* connection : _connections)
		xdelete(connection);

	_connections.clear();
}

DBConnection* DBConnectionPool::Pop()
{
	WRITE_LOCK;

	// conncection에 아무것도 없다면 null
	if (_connections.empty())
		return nullptr;

	// 아무거나 하나 꺼내서
	// return으로 뱉어주기
	DBConnection* connection = _connections.back();
	_connections.pop_back();
	return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
	WRITE_LOCK;
	_connections.push_back(connection);
}
