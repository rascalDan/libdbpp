#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include "command.h"
#include "command_fwd.h"
#include "mockdb.h"
#include <connection.h>
#include <exception>
#include <factory.impl.h>
#include <memory>
#include <optional>
#include <pq-command.h>
#include <vector>

BOOST_AUTO_TEST_CASE(create)
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	// MockDb is fake, just returns nullptr, but the call should otherwise succeed.
	BOOST_REQUIRE(!mock->modify(""));
	BOOST_REQUIRE(!mock->select(""));
}

BOOST_AUTO_TEST_CASE(resolve)
{
	auto libname = DB::Connection::resolvePlugin(typeid(DB::Connection), "postgresql");
	BOOST_REQUIRE(libname);
	BOOST_REQUIRE_EQUAL("libdbpp-postgresql.so", *libname);
	BOOST_REQUIRE_THROW(
			(void)DB::ConnectionFactory::createNew("otherdb", "doesn't matter"), AdHoc::LoadLibraryException);
}

BOOST_AUTO_TEST_CASE(finish)
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
}

BOOST_AUTO_TEST_CASE(tx)
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
}

BOOST_AUTO_TEST_CASE(txscope)
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	BOOST_REQUIRE(mock);
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	{
		DB::TransactionScope tx(*mock);
		BOOST_REQUIRE_EQUAL(true, mock->inTx());
	}
	BOOST_REQUIRE_EQUAL(false, mock->inTx());
	try {
		DB::TransactionScope tx(*mock);
		BOOST_REQUIRE_EQUAL(true, mock->inTx());
		throw std::exception();
	}
	catch (...) {
		BOOST_REQUIRE_EQUAL(false, mock->inTx());
	}
}

BOOST_AUTO_TEST_CASE(savepoints)
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	auto mockdb = std::dynamic_pointer_cast<MockDb>(mock);
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
}

BOOST_AUTO_TEST_CASE(commandOptions)
{
	auto optsDefault = DB::CommandOptionsFactory::createNew("", 1234, {});
	BOOST_REQUIRE(optsDefault);
	BOOST_REQUIRE(optsDefault->hash);
	BOOST_REQUIRE_EQUAL(1234, *optsDefault->hash);
}

BOOST_AUTO_TEST_CASE(commandOptionsPq1)
{
	auto optsBase = DB::CommandOptionsFactory::createNew("postgresql", 12345, {{"no-cursor", ""}, {"page-size", "5"}});
	BOOST_REQUIRE(optsBase);
	auto optsPq = std::dynamic_pointer_cast<PQ::CommandOptions>(optsBase);
	BOOST_REQUIRE(optsPq);
	BOOST_REQUIRE(optsBase->hash);
	BOOST_REQUIRE_EQUAL(12345, *optsBase->hash);
	BOOST_REQUIRE(!optsPq->useCursor);
	BOOST_REQUIRE_EQUAL(5, optsPq->fetchTuples);
}

BOOST_AUTO_TEST_CASE(commandOptionsPq2)
{
	auto optsBase = DB::CommandOptionsFactory::createNew("postgresql", 123456, {{"page-size", "50"}});
	BOOST_REQUIRE(optsBase);
	auto optsPq = std::dynamic_pointer_cast<PQ::CommandOptions>(optsBase);
	BOOST_REQUIRE(optsPq);
	BOOST_REQUIRE(optsBase->hash);
	BOOST_REQUIRE_EQUAL(123456, *optsBase->hash);
	BOOST_REQUIRE(optsPq->useCursor);
	BOOST_REQUIRE_EQUAL(50, optsPq->fetchTuples);
}
