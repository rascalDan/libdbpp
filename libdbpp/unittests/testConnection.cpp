#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include <factory.h>
#include <connection.h>
#include <pq-command.h>
#include <definedDirs.h>
#include <fstream>
#include <vector>
#include <error.h>
#include <sqlParse.h>
#include "mockdb.h"

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

BOOST_AUTO_TEST_CASE( finish )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	mock->beginTx();
	BOOST_REQUIRE_THROW(mock->finish(), DB::TransactionStillOpen);
	mock->rollbackTx();
	mock->finish();
	mock->beginTx();
	BOOST_REQUIRE_THROW(mock->finish(), DB::TransactionStillOpen);
	mock->commitTx();
	mock->finish();
	delete mock;
}

BOOST_AUTO_TEST_CASE( tx )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	mock->beginTx(); // 1
	BOOST_REQUIRE_EQUAL(true, mock->inTx());
	mock->beginTx(); // 2
	BOOST_REQUIRE_EQUAL(true, mock->inTx());
	mock->commitTx(); // 1
	BOOST_REQUIRE_EQUAL(true, mock->inTx());
	mock->beginTx(); // 2
	BOOST_REQUIRE_EQUAL(true, mock->inTx());
	mock->rollbackTx(); // 1
	BOOST_REQUIRE_EQUAL(true, mock->inTx());
	mock->rollbackTx(); // 0
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	delete mock;
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

BOOST_AUTO_TEST_CASE( commandOptions )
{
	auto optsDefault = DB::CommandOptionsFactory::createNew("", 1234, {});
	BOOST_REQUIRE(optsDefault);
	BOOST_REQUIRE(optsDefault->hash);
	BOOST_REQUIRE_EQUAL(1234, *optsDefault->hash);
}

BOOST_AUTO_TEST_CASE( commandOptionsPq )
{
	auto optsBase = DB::CommandOptionsFactory::createNew("postgresql", 1234, {});
	BOOST_REQUIRE(optsBase);
	auto optsPq = dynamic_cast<PQ::CommandOptions *>(optsBase);
	BOOST_REQUIRE(optsPq);
	BOOST_REQUIRE(optsBase->hash);
	BOOST_REQUIRE_EQUAL(1234, *optsBase->hash);
}

