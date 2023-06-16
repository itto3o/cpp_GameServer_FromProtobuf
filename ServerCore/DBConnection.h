#pragma once
#include <sql.h>
#include <sqlext.h>

/*-----------------
	DBConnection
------------------*/

enum 
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000
};
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
	// �̷��� �����ε��� ���� ��쿡�� �ϳ��� Ŭ���ؼ� �����θ� ����� ���ٴ�
	// Ŭ���� Ŭ��>alt shift q>create method implementation
	// �� �� �ȵ�....?
	bool			BindParam(int32 paramIndex, bool* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, float* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, double* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int8* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int16* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int32* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int64* value, SQLLEN* index);
												// unsigned�� DB��ǰ���� ���� unsigned��� ������ ����
												//--> �׳� �� ū ������ sigend�� �ؾ���
												// ĳ���� ������ ���� ������ ���� ��ȹ�� ����
												// boolean�� ������ int8¥���� ������

	bool			BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index);
	bool			BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index); // �̹���

	bool			BindCol(int32 columnIndex, bool* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, float* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, double* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, int8* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, int16* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, int32* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, int64* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool			BindCol(int32 columnIndex, const WCHAR* str, int32 size, SQLLEN* index);
	bool			BindCol(int32 columnIndex, const BYTE* bin, int32 size, SQLLEN* index);

private:
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

