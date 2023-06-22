#pragma once
#include "DBConnection.h"


template<int32 C>
				// value는 -1에다가 bitshift를 한 다음에 OR연산을 해서 fullbits에 C-1개만큼의 value를 추출
				// fullbits<3> = (1<<2) | fullbits<2> = (1<<2) | (1<<1) | (fullbits<0>)
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C - 1>::value }; };
// inputCount,		outputCount
// 이 두개의 카운트에 따라 다른 클래스가 만들어질 것이고, 서로의 연관성은 없음
// dbConn->BindParam(1, &gold, &len)); 경우는 3, 0
// 근데 1번부터 하고 있는데 1번부터 하면 좀 헷갈리는 것 같음 --> 0번부터

// 위와 예외적인 상황
template<>
struct FullBits<1> { enum { value = 1 }; };

template<>
struct FullBits<0> { enum { value = 0 }; };


template<int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query)
	{
		// 모든 초기화
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		dbConnection.Unbind();
	}

	bool Validate()
	{
		// 비트 플래그가 모두 채워져있는지 확인
		// 데이터 크기가 다르기 때문에 실시간으로 만들어서 해도 되지만
		// 개수만큼을 하나씩 계산해도 되지만,
		// 템플릿으로 굳이 만들어보자면
		return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query);
	}

	// 데이터가 여러개일 경우에 하나씩 긁어옴
	bool Fetch()
	{
		return _dbConnection.Fetch();
	}

public:
	// 바인딩하는 부분
	template<typename T>
	void BindParam(int32 idx, T& value)
	{
		// 실제 작성할땐 1번부터 넘겨야 하지만 
		// 우리가 할땐 0번부터 하는게 익숙하니까 1 더해서 넘겨주기
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
		// bit flag, 1을 bit shif만큼 옮겨서 OR연산
		// 만약 8개입력을 해야하면, 2번째 데이터를 인덱스로 넣었따고 가정하면,
		// 10이 켜짐, --> 설정한 개수 만큼이 모두 1로 켜졌으면 걔는 모든데이터를 채웠다고 인지
		// 8개면 11111111 인듯?
		_paramFlag |= (1LL << idx);
	}

	// 예외적인 경우, wchar 같은
	void BindParam(int32 idx, const WCHAR* value)
	{
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	// 일반 배열
	template<typename T, int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	// 포인터에 개수까지, 배열형태가 아니라 데이터를 지정해서 몇개짜리를 넣어줄때
	template<typename T>
	void BindParam(int32 idx, T* value, int32 N)
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	// bindColumn
	template<typename T>
	void BindCol(int32 idx, T& value)
	{
		_dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<int32 N>
	void BindCol(int32 idx, WCHAR(&value)[N])
	{									// nullbyte까지 생각해서 N-1만큼					
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	// 포인터랑 개수를 직접 지정할 때
	void BindCol(int32 idx, WCHAR* value, int32 len)
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	// 일반적인 바이트 배열
	template<typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N])
	{
		_dbConnection.BindCol(idx + 1, value, size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnection&		_dbConnection;

	// 실행해야할 쿼리
	const WCHAR*		_query;

	// 만약 ParamCount가 0이라면 배열을 못만들고 에러가 뜸,
	// ==> 0보다 작으면 1로
					// 이 문법이 되는 이유는 paramCount가 컴파일타임에 결정되기 때문
					// 런타임에 만들어지는 애로는 배열을 만들 수 없음
	SQLLEN				_paramIndex[ParamCount > 0 ? ParamCount : 1]; 
	SQLLEN				_columnIndex[ColumnCount > 0 ? ColumnCount : 1];

	// 모든 애들을 세팅했는지 알기 위해
	// 세팅한 개수 만큼을 추적하거나 bindParam할때마다 count+1할수도 있찌만
	// 실수로 같은 넘버를 두개 넣을땐 처리가 안되니까
	// 그런 부분까지 처리하기 위해 bitflag로 관리
	uint64				_paramFlag;
	uint64				_columnFlag;
};

