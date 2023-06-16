#pragma once
#include "DBConnection.h"

			// inputCount,		outputCount
			// 이 두개의 카운트에 따라 다른 클래스가 만들어질 것이고, 서로의 연관성은 없음
template<int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query)
	{

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
	SQLLEN				_columnIndex[ParamCount > 0 ? ParamCount : 1]; 

	// 모든 애들을 세팅했는지 알기 위해
	// 세팅한 개수 만큼을 추적하거나 bindParam할때마다 count+1할수도 있찌만
	// 실수로 같은 넘버를 두개 넣을땐 처리가 안되니까
	// 그런 부분까지 처리하기 위해 bitflag로 관리
	uint64				_paramFlag;
	uint64				_columnFlag;
};

