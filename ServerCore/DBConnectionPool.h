#pragma once
#include "DBConnection.h"

/*----------------------
	DBConnectionPool
------------------------*/
// �������� �� �ϳ��� ��� ���� ��
// � ������� �̰� �ʿ����� ������ �� ��� �������� ���� �ʿ��� �ְ� ������
// ==> CoreGlobal.h�� �־��ֱ�
// ���� DB�� �����ϴ� ��쿣 ������ �ܿ��� ���������� �����ϴ°� ����
class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	// connection�� ������ִ� ��
	// connection�� ���� ����, Ŀ�ؼ� ���鶧 ����(� ���, � ȯ��, � ���� ���)
	bool				Connect(int32 connectionCount, const WCHAR* connectionString);
	void				Clear();

	// �긦 shared_ptr�� ���� �ʿ�� ���°� �ܼ��ϰ� pop�� �Ŀ� �ٽ� �ٷ� �ݳ��� ����
	DBConnection*		Pop();
	void				Push(DBConnection* connection);

private:
	USE_LOCK;
	SQLHENV					_environment = SQL_NULL_HANDLE; //Handle ENV, ȯ�� ����ϴ� Handle
	Vector<DBConnection*>	_connections;
};

