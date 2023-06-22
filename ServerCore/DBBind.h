#pragma once
#include "DBConnection.h"


template<int32 C>
				// value�� -1���ٰ� bitshift�� �� ������ OR������ �ؼ� fullbits�� C-1����ŭ�� value�� ����
				// fullbits<3> = (1<<2) | fullbits<2> = (1<<2) | (1<<1) | (fullbits<0>)
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C - 1>::value }; };
// inputCount,		outputCount
// �� �ΰ��� ī��Ʈ�� ���� �ٸ� Ŭ������ ������� ���̰�, ������ �������� ����
// dbConn->BindParam(1, &gold, &len)); ���� 3, 0
// �ٵ� 1������ �ϰ� �ִµ� 1������ �ϸ� �� �򰥸��� �� ���� --> 0������

// ���� �������� ��Ȳ
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
		// ��� �ʱ�ȭ
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		dbConnection.Unbind();
	}

	bool Validate()
	{
		// ��Ʈ �÷��װ� ��� ä�����ִ��� Ȯ��
		// ������ ũ�Ⱑ �ٸ��� ������ �ǽð����� ���� �ص� ������
		// ������ŭ�� �ϳ��� ����ص� ������,
		// ���ø����� ���� �����ڸ�
		return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query);
	}

	// �����Ͱ� �������� ��쿡 �ϳ��� �ܾ��
	bool Fetch()
	{
		return _dbConnection.Fetch();
	}

public:
	// ���ε��ϴ� �κ�
	template<typename T>
	void BindParam(int32 idx, T& value)
	{
		// ���� �ۼ��Ҷ� 1������ �Ѱܾ� ������ 
		// �츮�� �Ҷ� 0������ �ϴ°� �ͼ��ϴϱ� 1 ���ؼ� �Ѱ��ֱ�
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
		// bit flag, 1�� bit shif��ŭ �Űܼ� OR����
		// ���� 8���Է��� �ؾ��ϸ�, 2��° �����͸� �ε����� �־����� �����ϸ�,
		// 10�� ����, --> ������ ���� ��ŭ�� ��� 1�� �������� �´� ��絥���͸� ä���ٰ� ����
		// 8���� 11111111 �ε�?
		_paramFlag |= (1LL << idx);
	}

	// �������� ���, wchar ����
	void BindParam(int32 idx, const WCHAR* value)
	{
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	// �Ϲ� �迭
	template<typename T, int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	// �����Ϳ� ��������, �迭���°� �ƴ϶� �����͸� �����ؼ� �¥���� �־��ٶ�
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
	{									// nullbyte���� �����ؼ� N-1��ŭ					
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	// �����Ͷ� ������ ���� ������ ��
	void BindCol(int32 idx, WCHAR* value, int32 len)
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	// �Ϲ����� ����Ʈ �迭
	template<typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N])
	{
		_dbConnection.BindCol(idx + 1, value, size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
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
	SQLLEN				_columnIndex[ColumnCount > 0 ? ColumnCount : 1];

	// ��� �ֵ��� �����ߴ��� �˱� ����
	// ������ ���� ��ŭ�� �����ϰų� bindParam�Ҷ����� count+1�Ҽ��� ���
	// �Ǽ��� ���� �ѹ��� �ΰ� ������ ó���� �ȵǴϱ�
	// �׷� �κб��� ó���ϱ� ���� bitflag�� ����
	uint64				_paramFlag;
	uint64				_columnFlag;
};

