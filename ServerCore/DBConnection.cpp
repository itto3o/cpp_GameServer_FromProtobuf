#include "pch.h"
#include "DBConnection.h"

/*-----------------
	DBConnection
------------------*/
bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	// ��¥�� DB�� ����
	// handle�� �Ҵ�, SQLHDBC�� connectionHandle�� �������
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS)
		return false;

	// connection string�� �̿��ؼ� ���������� DB����
	WCHAR stringBuffer[MAX_PATH] = { 0 };
	// wcscpy_s ���ڿ� ���� �Լ�(WCHAR�����ε�)
	::wcscpy_s(stringBuffer, connectionString);

	// ���� ����� ���� ����
	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0; // resultString�� ����

	// wchar������ �̿��ؼ� ����
	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT & resultStringLen,
		SQL_DRIVER_NOPROMPT
	);

	// statement ���� �����
	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
		return false;

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnection::Clear()
{
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query)
{
	// query�� �޾Ƽ� �¸� �����ش޶�
	// statement�� �̿��ؼ� �������� ���ڵ��� ���� ������ ��
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}

bool DBConnection::Fetch()
{
	// �����͸� �ܾ�� �� ���

	// �����ߴٸ� �޾ƿ� �����Ͱ� �ִٴ� ��
	SQLRETURN ret = ::SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		// �����Ͱ� ���ٸ�
		// ������ ���������� ��ȯ�ϴ� ���� ���� ���ٸ� nodata�� ��
		return false;
	case SQL_ERROR:
		// ��û�� �߸��ƴٸ� error
		HandleError(ret);
		return false;
	default:
		return true;
	}
}

int32 DBConnection::GetRowCount()
{
	// �����Ͱ� � �ִ���
	// statement�� ���ڸ� �ѱ涧�� ���, ���� �� ���ڵ��� �޾ƿ� ���� ���
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT &count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::unbind()
{
	// ���� bind param, bind col�Լ��� ���� ���ڵ��� �޾ƿ��⵵ �ϰ� �Ѱ��ֱ⵵ ���ٵ�
	// ó���� ����Ҷ��� ������ ���εǾ��� �����Ͱ� ���� �� �����ϱ� �װ͵��� ���������
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
	// ���° ���ڸ� �������� �����ϰ� �������� �־��ֱ�									// �����Ͱ� �ִ� ���� pointer�� �Ѱ��ֱ�, index�� ���������� ���
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

bool DBConnection::BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	// �����̶�� �߸��°Ŵϱ� return
	if (ret == SQL_SUCCESS)
		return;

	// � �������� �˾ƺ��� ���� ���ڵ�
	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;

	// � ������ ���� �������� �޽��� �޾ƿ�
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorRet = 0;

	while (true)
	{
		// error �޽��� ����
		errorRet = ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT &nativeErr,
			errMsg,
			_countof(errMsg),
			OUT &msgLen
		);

		// �ƹ��� ������ ������ return
		if (errorRet == SQL_NO_DATA)
			break;

		// success���ٸ� ���ٸ� ���� x
		// == ||���� �ϴ°� �ƴѰ� sql_success�� with info�� �Ѵ� �ƴϸ� break..?
		if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
			break;

		// TODO : Log
		// �ش��ϴ� �����޽����� ��������
		// �ѱ��� ȯ�浵 ���Ե� �� �����ϱ� �ѱ���� ����
		wcout.imbue(locale("kor"));
		wcout << errMsg << endl; // ���߿� �������������

		index++;
	}
}
