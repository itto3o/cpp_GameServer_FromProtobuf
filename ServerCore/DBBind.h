#pragma once
#include "DBConnection.h"

			// inputCount,		outputCount
			// �� �ΰ��� ī��Ʈ�� ���� �ٸ� Ŭ������ ������� ���̰�, ������ �������� ����
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

	// �����ؾ��� ����
	const WCHAR*		_query;

	// ���� ParamCount�� 0�̶�� �迭�� ������� ������ ��,
	// ==> 0���� ������ 1��
					// �� ������ �Ǵ� ������ paramCount�� ������Ÿ�ӿ� �����Ǳ� ����
					// ��Ÿ�ӿ� ��������� �ַδ� �迭�� ���� �� ����
	SQLLEN				_paramIndex[ParamCount > 0 ? ParamCount : 1]; 
	SQLLEN				_columnIndex[ParamCount > 0 ? ParamCount : 1]; 

	// ��� �ֵ��� �����ߴ��� �˱� ����
	// ������ ���� ��ŭ�� �����ϰų� bindParam�Ҷ����� count+1�Ҽ��� ���
	// �Ǽ��� ���� �ѹ��� �ΰ� ������ ó���� �ȵǴϱ�
	// �׷� �κб��� ó���ϱ� ���� bitflag�� ����
	uint64				_paramFlag;
	uint64				_columnFlag;
};

