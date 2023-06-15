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

	bool			Execute(const WCHAR* query); // sql 쿼리를 실행하는 함수
	bool			Fetch(); // 결과를 받아오는 애랑 관련
	int32			GetRowCount();
	void			unbind();

public:
	// sql qeury할때 statment를 통해서 인자들을 넘겨주기 위함
	// 인자가 여러개일 수 있으니까 parameter의 순서를 같이, ctype은 c데이터 형식,
	// ctype : ODBC는 c++만 지원하는건 아니니까 자료형에 따라 ctype, sqltype형식이 다 있음,
	bool			BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	
	// 쿼리를 실행한 후 데이터를 긁어올 때(받아올 때 사용하는 인자들)
	// 어떤 메모리 주소를 건네줘서 거기에 데이터를 받아와야 함
	bool			BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);

	// error가 났을때 공용으로 관리
	void			HandleError(SQLRETURN ret);


private:
	// 핸들 두개,
	// SQLHDBC : db connection handle
	SQLHDBC			_connection = SQL_NULL_HANDLE;

	// HSTMT : statment handle
	SQLHSTMT		_statement = SQL_NULL_HANDLE; //요청보낼때 statment를 보냄
};

