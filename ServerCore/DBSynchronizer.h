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

	// 서로 의존하고 있으면 위험할테니까
	// 별로 위험하지 않은 순서대로 기입, 조금 바꿔도 문제가 일어나진 않음
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

	// 얘를 호출해주기만 하면 끝
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

	// xml에서 설계한 디비구조
	Vector<DBModel::TableRef>			_xmlTables;
	Vector<DBModel::ProcedureRef>		_xmlProcedures;
	// 삭제해야하는 애들도 생길텐데 xml에 그 테이블 정보가 없다고 해도
	// 당장 바로 삭제하면 위험할 수 있을테니까,
	// 진짜로 삭제되기 원하다면 xml에 remove 태그를 넣기
	Set<String>							_xmlRemovedTables; 

	// db의 현재 상태를 들고있음
	Vector<DBModel::TableRef>			_dbTables;
	Vector<DBModel::ProcedureRef>		_dbProcedures;

private:
	Set<String>							_dependentIndexes;
	Vector<String>						_updateQueries[UpdateStep::Max];
};