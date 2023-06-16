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

	bool			Execute(const WCHAR* query); // sql 쿼리를 실행하는 함수
	bool			Fetch(); // 결과를 받아오는 애랑 관련
	int32			GetRowCount();
	void			unbind();

public:
	// 이렇게 오버로딩이 많을 경우에는 하나씩 클릭해서 구현부를 만들기 보다는
	// 클래스 클릭>alt shift q>create method implementation
	// 왜 난 안돼....?
	bool			BindParam(int32 paramIndex, bool* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, float* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, double* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int8* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int16* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int32* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, int64* value, SQLLEN* index);
												// unsigned는 DB제품군에 따라 unsigned라는 개념이 없음
												//--> 그냥 더 큰 영역에 sigend를 해야함
												// 캐스팅 문제로 인해 지원을 해줄 계획이 없대
												// boolean도 없지만 int8짜리로 만들면됨

	bool			BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool			BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index);
	bool			BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index); // 이미지

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

