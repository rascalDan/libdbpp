#include "mockDatabase.h"
#include <buffer.h>
#include <fstream>
#include <modifycommand.h>
#include <plugins.impl.h>

namespace AdHoc {
	template <>
	PluginOf<DB::MockDatabase>::~PluginOf()
	{
		// This implementation doesn't delete .implementation as
		// mock databases simply unregister themselves (via destructor)
		// when the mock framework tears them down.
	}
}
INSTANIATEPLUGINOF(DB::MockDatabase);

namespace DB {

unsigned int MockDatabase::mocked = 0;

MockDatabase::MockDatabase(const std::string & name) :
	mockName(name)
{
	AdHoc::PluginManager::getDefault()->add(AdHoc::PluginPtr(new AdHoc::PluginOf<MockDatabase>(this, mockName, __FILE__, __LINE__)));
}

MockDatabase::~MockDatabase()
{
	AdHoc::PluginManager::getDefault()->remove<MockDatabase>(mockName);
}

Connection *
MockDatabase::openConnectionTo(const std::string & mockName)
{
	return AdHoc::PluginManager::getDefault()->get<DB::MockDatabase>(mockName)->implementation()->openConnection();
}

void
MockDatabase::PlaySchemaScripts(const std::vector<boost::filesystem::path> & ss) const
{
	DB::Connection * conn = openConnection();
	try {
		CreateStatusTable(conn);
		for (auto s : ss) {
			conn->beginTx();
			PlaySchemaScript(conn, s);
			conn->commitTx();
		}
		DropStatusTable(conn);
		delete conn;
	}
	catch (...) {
		if (conn->inTx()) {
			conn->rollbackTx();
		}
		delete conn;
		DropDatabase();
		throw;
	}
}

void
MockDatabase::PlaySchemaScript(DB::Connection * conn, const boost::filesystem::path & s) const
{
	UpdateStatusTable(conn, s);
	std::ifstream f;
	f.open(s.string());
	if (!f.good()) {
		throw std::runtime_error("Failed to open mock script: " + s.string());
	}
	conn->executeScript(f, s.parent_path());
	f.close();
}

void
MockDatabase::CreateStatusTable(DB::Connection * conn) const
{
	conn->execute(
			"CREATE TABLE _p2_teststatus( \
				pid int, \
				script varchar(256), \
				scriptdir varchar(256))");
	auto ins = conn->newModifyCommand(
			"INSERT INTO _p2_teststatus(pid) VALUES(?)");
	ins->bindParamI(0, getpid());
	ins->execute();
	delete ins;
}

void
MockDatabase::DropStatusTable(DB::Connection * conn) const
{
	conn->execute("DROP TABLE _p2_teststatus");
}

void
MockDatabase::UpdateStatusTable(DB::Connection * conn, const boost::filesystem::path & script) const
{
	auto upd = conn->newModifyCommand(
			"UPDATE _p2_teststatus SET script = ?, scriptdir = ?");
	upd->bindParamS(0, script.string());
	upd->bindParamS(1, script.parent_path().string());
	upd->execute();
	delete upd;
}

MockServerDatabase::MockServerDatabase(const std::string & masterdb, const std::string & name, const std::string & type) :
	MockDatabase(name),
	master(DB::ConnectionFactory::create(type, masterdb)),
	testDbName(stringbf("test_%d_%d", getpid(), ++mocked))
{
}

MockServerDatabase::~MockServerDatabase()
{
	delete master;
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

