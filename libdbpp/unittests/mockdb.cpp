#include "mockdb.h"

// LCOV_EXCL_START

MockDb::MockDb(const std::string &) { }

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
MockDb::execute(const std::string & sql, const DB::CommandOptionsCPtr &)
{
	if (sql.substr(0, 3) == "Not") {
		throw DB::Error();
	}
	executed.push_back(sql);
}

DB::SelectCommandPtr
MockDb::select(const std::string &, const DB::CommandOptionsCPtr &)
{
	return nullptr;
}

DB::ModifyCommandPtr
MockDb::modify(const std::string &, const DB::CommandOptionsCPtr &)
{
	return nullptr;
}

MockMock::MockMock(const std::string &, const std::string &, const std::vector<std::filesystem::path> & ss) :
	DB::MockDatabase()
{
	MockMock::CreateNewDatabase();
	PlaySchemaScripts(ss);
}

DB::ConnectionPtr
MockMock::openConnection() const
{
	return std::make_shared<MockDb>("");
}

void
MockMock::CreateNewDatabase() const
{
}

void
MockMock::DropDatabase() const
{
}

FACTORY(MockMock, DB::MockDatabaseFactory)
FACTORY(MockDb, DB::ConnectionFactory)

// LCOV_EXCL_STOP
