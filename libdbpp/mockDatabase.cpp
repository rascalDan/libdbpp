#define BOOST_FILESYSTEM_DYN_LINK
#define BOOST_FILESYSTEM_SOURCE
#include "mockDatabase.h"
#include <compileTimeFormatter.h>
#include <fstream>
#include <modifycommand.h>
#include <plugins.impl.h>
#include <factory.impl.h>

INSTANTIATEPLUGINOF(DB::MockDatabase);
INSTANTIATEFACTORY(DB::MockDatabase, const std::string &, const std::string &, const std::vector<std::filesystem::path> &);
PLUGINRESOLVER(DB::MockDatabaseFactory, DB::Connection::resolvePlugin);

namespace DB {

unsigned int MockDatabase::mocked = 0;


ConnectionPtr
MockDatabase::openConnectionTo(const std::string & mockName)
{
	return AdHoc::PluginManager::getDefault()->get<DB::MockDatabase>(mockName)->implementation()->openConnection();
}

void
MockDatabase::PlaySchemaScripts(const std::vector<std::filesystem::path> & ss) const
{
	auto conn = ConnectionPtr(openConnection());
	try {
		for (auto s : ss) {
			conn->beginTx();
			PlaySchemaScript(conn.get(), s);
			conn->commitTx();
		}
	}
	catch (...) {
		if (conn->inTx()) {
			conn->rollbackTx();
		}
		DropDatabase();
		throw;
	}
}

void
MockDatabase::PlaySchemaScript(DB::Connection * conn, const std::filesystem::path & s) const
{
	std::ifstream f;
	f.open(s);
	if (!f.good()) {
		throw std::fstream::failure("Failed to open mock script: " + s.string());
	}
	conn->executeScript(f, s.parent_path());
	f.close();
}

AdHocFormatter(MockServerDatabaseName, "libdbpp_mock_%?_%?_%?");
MockServerDatabase::MockServerDatabase(const std::string & masterdb, const std::string & name, const std::string & type) :
	master(DB::ConnectionFactory::createNew(type, masterdb)),
	testDbName(MockServerDatabaseName::get(name, getpid(), ++mocked))
{
}

const std::string &
MockServerDatabase::databaseName() const
{
	return testDbName;
}

void
MockServerDatabase::CreateNewDatabase() const
{
	DropDatabase();
	master->execute("CREATE DATABASE " + testDbName);
}

void
MockServerDatabase::DropDatabase() const
{
	master->execute("DROP DATABASE IF EXISTS " + testDbName);
}

}

