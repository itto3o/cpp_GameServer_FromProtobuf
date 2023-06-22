#pragma once
#include "DBConnection.h"
#include "DBModel.h"

/*--------------------
	DBSynchronizer
---------------------*/

class DBSynchronizer
{
	enum
	{
		PROCEDURE_MAX_LEN = 10000
	};

	// ���� �����ϰ� ������ �������״ϱ�
	// ���� �������� ���� ������� ����, ���� �ٲ㵵 ������ �Ͼ�� ����
	enum UpdateStep : uint8
	{
		DropIndex,
		AlterColumn,
		AddColumn,
		CreateTable,
		DefaultConstraint,
		CreateIndex,
		DropColumn,
		DropTable,
		StoredProcecure,

		Max
	};

	enum ColumnFlag : uint8
	{
		Type = 1 << 0,
		Nullable = 1 << 1,
		Identity = 1 << 2,
		Default = 1 << 3,
		Length = 1 << 4,
	};

public:
	DBSynchronizer(DBConnection& conn) : _dbConn(conn) { }
	~DBSynchronizer();

	// �긦 ȣ�����ֱ⸸ �ϸ� ��
	bool		Synchronize(const WCHAR* path);

private:
	void		ParseXmlDB(const WCHAR* path);
	bool		GatherDBTables();
	bool		GatherDBIndexes();
	bool		GatherDBStoredProcedures();

	void		CompareDBModel();
	void		CompareTables(DBModel::TableRef dbTable, DBModel::TableRef xmlTable);
	void		CompareColumns(DBModel::TableRef dbTable, DBModel::ColumnRef dbColumn, DBModel::ColumnRef xmlColumn);
	void		CompareStoredProcedures();

	void		ExecuteUpdateQueries();

private:
	DBConnection& _dbConn;

	// xml���� ������ �����
	Vector<DBModel::TableRef>			_xmlTables;
	Vector<DBModel::ProcedureRef>		_xmlProcedures;
	// �����ؾ��ϴ� �ֵ鵵 �����ٵ� xml�� �� ���̺� ������ ���ٰ� �ص�
	// ���� �ٷ� �����ϸ� ������ �� �����״ϱ�,
	// ��¥�� �����Ǳ� ���ϴٸ� xml�� remove �±׸� �ֱ�
	Set<String>							_xmlRemovedTables; 

	// db�� ���� ���¸� �������
	Vector<DBModel::TableRef>			_dbTables;
	Vector<DBModel::ProcedureRef>		_dbProcedures;

private:
	Set<String>							_dependentIndexes;
	Vector<String>						_updateQueries[UpdateStep::Max];
};