#include "mockdb.h"

// LCOV_EXCL_START

MockDb::MockDb(const std::string &)
{
}

void
MockDb::beginTxInt()
{
}

void
MockDb::commitTxInt()
{
}
void
MockDb::rollbackTxInt()
{
}
void
MockDb::ping() const
{

}

DB::BulkDeleteStyle
MockDb::bulkDeleteStyle() const
{
	return DB::BulkDeleteUsingUsing;
}

DB::BulkUpdateStyle
MockDb::bulkUpdateStyle() const
{
	return DB::BulkUpdateUsingJoin;
}

void
MockDb::execute(const std::string & sql, const DB::CommandOptions *)
{
	if (sql.substr(0, 3) == "Not") {
		throw DB::Error();
	}
	executed.push_back(sql);
}

DB::SelectCommand *
MockDb::newSelectCommand(const std::string &, const DB::CommandOptions *)
{
	return nullptr;
}

DB::ModifyCommand *
MockDb::newModifyCommand(const std::string &, const DB::CommandOptions *)
{
	return nullptr;
}

MockMock::MockMock(const std::string &, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	DB::MockDatabase(name)
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
}

DB::Connection *
MockMock::openConnection() const
{
	return new MockDb("");
}

void
MockMock::CreateNewDatabase() const
{
}

void
MockMock::DropDatabase() const
{
}

FACTORY(MockMock, DB::MockDatabaseFactory);
FACTORY(MockDb, DB::ConnectionFactory);

// LCOV_EXCL_STOP

