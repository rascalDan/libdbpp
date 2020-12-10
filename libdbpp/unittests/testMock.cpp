#define BOOST_TEST_MODULE DbMock
#include <boost/test/unit_test.hpp>

#include "../error.h"
#include "../mockDatabase.h"
#include "mockdb.h"
#include <definedDirs.h>
#include <fstream>

BOOST_AUTO_TEST_CASE(noFactory)
{
	BOOST_REQUIRE_THROW({ (void)DB::MockDatabaseFactory::get("not-found"); }, AdHoc::LoadLibraryException);
}

BOOST_AUTO_TEST_CASE(mockFactory)
{
	const auto f = DB::MockDatabaseFactory::get("MockMock");
	BOOST_REQUIRE(f);
	const auto m = f->create("", typeid(this).name(), {});
	BOOST_REQUIRE(m);
	const auto c = m->openConnection();
	BOOST_REQUIRE(c);
	const auto & cr = *c;
	BOOST_REQUIRE_EQUAL(typeid(MockDb), typeid(cr));
}

BOOST_AUTO_TEST_CASE(missingMock)
{
	BOOST_REQUIRE_THROW(
			{
				(void)DB::MockDatabaseFactory::createNew(
						"MockMock", "user=postgres dbname=postgres", typeid(this).name(), {rootDir / "missing.sql"});
			},
			std::fstream::failure);
}

BOOST_AUTO_TEST_CASE(failingMock)
{
	BOOST_REQUIRE_THROW(
			{
				(void)DB::MockDatabaseFactory::createNew(
						"MockMock", "user=postgres dbname=postgres", typeid(this).name(), {rootDir / "badMock.sql"});
			},
			DB::Error);
}
