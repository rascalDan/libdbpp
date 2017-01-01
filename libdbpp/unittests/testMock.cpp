#define BOOST_TEST_MODULE DbMock
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <fstream>
#include "../error.h"
#include "../mockDatabase.h"
#include "mockdb.h"

BOOST_AUTO_TEST_CASE( noFactory )
{
	BOOST_REQUIRE_THROW({
		DB::MockDatabaseFactory::get("not-found");
	}, AdHoc::NoSuchPluginException);
}

BOOST_AUTO_TEST_CASE( mockFactory )
{
	auto f = DB::MockDatabaseFactory::get("MockMock");
	BOOST_REQUIRE(f);
	auto m = f->create("", typeid(this).name(), {});
	BOOST_REQUIRE(m);
	auto c = m->openConnection();
	BOOST_REQUIRE(c);
	BOOST_REQUIRE_EQUAL(typeid(MockDb), typeid(*c));
}

BOOST_AUTO_TEST_CASE( missingMock )
{
	BOOST_REQUIRE_THROW({
		DB::MockDatabaseFactory::createNew("MockMock",
				"user=postgres dbname=postgres", typeid(this).name(), { rootDir / "missing.sql" });
	}, std::fstream::failure);
}

BOOST_AUTO_TEST_CASE( failingMock )
{
	BOOST_REQUIRE_THROW({
		DB::MockDatabaseFactory::createNew("MockMock",
				"user=postgres dbname=postgres", typeid(this).name(), { rootDir / "badMock.sql" });
	}, DB::Error);
}

