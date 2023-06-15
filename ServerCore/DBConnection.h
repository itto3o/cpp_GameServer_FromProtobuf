#pragma once
#include <sql.h>
#include <sqlext.h>

/*-----------------
	DBConnection
------------------*/
class DBConnection
{
public:
	bool			Connect(SQLHENV henv, const WCHAR* connectionString);
	void			Clear();

	bool			Execute(const WCHAR* query); // sql ������ �����ϴ� �Լ�
	bool			Fetch(); // ����� �޾ƿ��� �ֶ� ����
	int32			GetRowCount();
	void			unbind();

public:
	// sql qeury�Ҷ� statment�� ���ؼ� ���ڵ��� �Ѱ��ֱ� ����
	// ���ڰ� �������� �� �����ϱ� parameter�� ������ ����, ctype�� c������ ����,
	// ctype : ODBC�� c++�� �����ϴ°� �ƴϴϱ� �ڷ����� ���� ctype, sqltype������ �� ����,
	bool			BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	
	// ������ ������ �� �����͸� �ܾ�� ��(�޾ƿ� �� ����ϴ� ���ڵ�)
	// � �޸� �ּҸ� �ǳ��༭ �ű⿡ �����͸� �޾ƿ;� ��
	bool			BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);

	// error�� ������ �������� ����
	void			HandleError(SQLRETURN ret);


private:
	// �ڵ� �ΰ�,
	// SQLHDBC : db connection handle
	SQLHDBC			_connection = SQL_NULL_HANDLE;

	// HSTMT : statment handle
	SQLHSTMT		_statement = SQL_NULL_HANDLE; //��û������ statment�� ����
};

