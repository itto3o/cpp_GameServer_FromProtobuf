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
	// ���� �ξ��ֱ�, ó�� ������ �㶧 �� �ѹ���
	WRITE_LOCK;

	// ��� ������ ����ϴ��� ��� �����ְԵ�
	// ó������ ENV�� �������� ���̴�. �ι�°�� NULL, ����°�� environment��ü
	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment) != SQL_SUCCESS)
		return false;

	// handle�� �̿��� ODBC ������ �������� �� ����, 3���� ����
	if (::SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		return false;

	// conncection count ������ŭ ������ �ξ��ֱ�
	// DBConnection ��ü�� ��û�� ���� ����
	// �� �����常 ��û�� �Ѵٰ� ������x,
	// �̷� ��� connectioncount�� ������ ������ŭ ���� �ϳ��� ��������Ѵٰ� ���� ��
	for (int32 i = 0; i < connectionCount; i++)
	{
		DBConnection* connection = xnew<DBConnection>();

		// ȯ���� ���ؼ� ���������� conncection�� ����������
		if (connection->Connect(_environment, connectionString) == false)
			return false;

		_connections.push_back(connection);
	}

	return true;
}

void DBConnectionPool::Clear()
{
	WRITE_LOCK;
	
	// ���� �۾�
	if (_environment != SQL_NULL_HANDLE) // null�� �ƴ϶�� free
	{
		// ���� � �ָ� �ݾ��ְ� �ִ��� handle type�� ���� �����ְ� ����
		::SQLFreeHandle(SQL_HANDLE_ENV, _environment);
		_environment = SQL_NULL_HANDLE;
	}

	// connection�� �ϳ��� �� �޸𸮸� �����ֱ�
	for (DBConnection* connection : _connections)
		xdelete(connection);

	_connections.clear();
}

DBConnection* DBConnectionPool::Pop()
{
	WRITE_LOCK;

	// conncection�� �ƹ��͵� ���ٸ� null
	if (_connections.empty())
		return nullptr;

	// �ƹ��ų� �ϳ� ������
	// return���� ����ֱ�
	DBConnection* connection = _connections.back();
	_connections.pop_back();
	return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
	WRITE_LOCK;
	_connections.push_back(connection);
}
