#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include <factory.h>
#include <connection.h>
#include <definedDirs.h>
#include <fstream>
#include <vector>
#include <error.h>
#include <sqlParse.h>

// LCOV_EXCL_START
class MockDb : public DB::Connection {
	public:
		MockDb(const std::string &) : txDepth(0) {}

		void	finish() const override {}
		int		beginTx() const override { return ++txDepth; }
		int		commitTx() const override { return --txDepth; }
		int		rollbackTx() const override { return --txDepth; }
		bool	inTx() const override { return txDepth > 0; }
		void	ping() const override {}
		DB::BulkDeleteStyle bulkDeleteStyle() const override { return DB::BulkDeleteUsingUsing; }
		DB::BulkUpdateStyle bulkUpdateStyle() const override { return DB::BulkUpdateUsingJoin; }

		void execute(const std::string & sql) override {
			executed.push_back(sql);
		}
		DB::SelectCommand * newSelectCommand(const std::string &) override { return nullptr; }
		DB::ModifyCommand * newModifyCommand(const std::string &) override { return nullptr; }

		mutable std::vector<std::string> executed;
		mutable unsigned int txDepth;
};
// LCOV_EXCL_STOP

FACTORY(MockDb, DB::ConnectionFactory);

BOOST_AUTO_TEST_CASE( plugins )
{
	auto pm = AdHoc::PluginManager::getDefault();
	BOOST_REQUIRE(pm);
	BOOST_REQUIRE_EQUAL(1, pm->count());
	BOOST_REQUIRE_EQUAL(1, pm->getAll().size());
	BOOST_REQUIRE_EQUAL(1, pm->getAll<DB::ConnectionFactory>().size());
}

BOOST_AUTO_TEST_CASE( create )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	// MockDb is fake, just returns nullptr, but the call should otherwise succeed.
	BOOST_REQUIRE(!mock->newModifyCommand(""));
	BOOST_REQUIRE(!mock->newSelectCommand(""));
	delete mock;
}

BOOST_AUTO_TEST_CASE( resolve )
{
	auto libname = DB::Connection::resolvePlugin(typeid(DB::Connection), "postgresql");
	BOOST_REQUIRE(libname);
	BOOST_REQUIRE_EQUAL("libdbpp-postgresql.so", *libname);
	BOOST_REQUIRE_THROW(DB::ConnectionFactory::createNew("otherdb", "doesn't matter"), AdHoc::LoadLibraryException);
}

BOOST_AUTO_TEST_CASE( parseBad )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	std::fstream s("/bad");
	BOOST_REQUIRE_THROW(mock->executeScript(s, rootDir), DB::SqlParseException);
	delete mock;
}

BOOST_AUTO_TEST_CASE( parse )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	std::fstream s((rootDir / "parseTest.sql").string());
	mock->executeScript(s, rootDir);
	MockDb * mockdb = dynamic_cast<MockDb *>(mock);
	BOOST_REQUIRE(mockdb);
	BOOST_REQUIRE_EQUAL(3, mockdb->executed.size());
	BOOST_REQUIRE_EQUAL("INSERT INTO name(t, i) VALUES('string', 3)", mockdb->executed[1]);
	delete mock;
}

BOOST_AUTO_TEST_CASE( parse2 )
{
	auto mock = DB::ConnectionPtr(DB::ConnectionFactory::createNew("MockDb", "doesn't matter"));
	auto mockdb = boost::dynamic_pointer_cast<MockDb>(mock);
	BOOST_REQUIRE(mockdb);
	std::ifstream s;

	s.open((rootDir / "dollarQuote.sql").string());
	mock->executeScript(s, rootDir);
	s.close();

	s.open((rootDir / "scriptDir.sql").string());
	mock->executeScript(s, rootDir);
	s.close();

	s.open((rootDir / "stringParse.sql").string());
	mock->executeScript(s, rootDir);
	s.close();
	BOOST_REQUIRE_EQUAL(4, mockdb->executed.size());
	BOOST_REQUIRE_EQUAL("INSERT INTO name(t, i) VALUES('fancy string '' \\' \\r \\n', 7)", mockdb->executed[3]);

	BOOST_REQUIRE_THROW({
			s.open((rootDir / "unterminatedComment.sql").string());
			mock->executeScript(s, rootDir);
		}, DB::SqlParseException);
	s.close();

	BOOST_REQUIRE_THROW({
			s.open((rootDir / "unterminatedDollarQuote.sql").string());
			mock->executeScript(s, rootDir);
		}, DB::SqlParseException);
	s.close();

	BOOST_REQUIRE_THROW({
			s.open((rootDir / "unterminatedString.sql").string());
			mock->executeScript(s, rootDir);
		}, DB::SqlParseException);
	s.close();
}

BOOST_AUTO_TEST_CASE( txscope )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	{
		DB::TransactionScope tx(mock);
		BOOST_REQUIRE_EQUAL(true, mock->inTx());
	}
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	try {
		DB::TransactionScope tx(mock);
		BOOST_REQUIRE_EQUAL(true, mock->inTx());
		throw std::exception();
	}
	catch (...) {
		BOOST_REQUIRE_EQUAL(false, mock->inTx());
	}
}

BOOST_AUTO_TEST_CASE( savepoints )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	MockDb * mockdb = dynamic_cast<MockDb *>(mock);
	BOOST_REQUIRE(mockdb);
	mock->savepoint("sp");
	BOOST_REQUIRE_EQUAL("SAVEPOINT sp", *mockdb->executed.rbegin());
	mock->releaseSavepoint("sp");
	BOOST_REQUIRE_EQUAL("RELEASE SAVEPOINT sp", *mockdb->executed.rbegin());
	mock->savepoint("sp1");
	BOOST_REQUIRE_EQUAL("SAVEPOINT sp1", *mockdb->executed.rbegin());
	mock->savepoint("sp2");
	BOOST_REQUIRE_EQUAL("SAVEPOINT sp2", *mockdb->executed.rbegin());
	mock->rollbackToSavepoint("sp1");
	BOOST_REQUIRE_EQUAL("ROLLBACK TO SAVEPOINT sp1", *mockdb->executed.rbegin());
	delete mock;
}

