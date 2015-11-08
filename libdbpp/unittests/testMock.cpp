#define BOOST_TEST_MODULE DbMock
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <fstream>
#include <mock.h>
#include "error.h"

BOOST_AUTO_TEST_CASE( missingMock )
{
	BOOST_REQUIRE_THROW({
			PQ::Mock m ("user=postgres dbname=postgres", "pqmock", { rootDir / "missing.sql" });
		}, std::fstream::failure);
}

BOOST_AUTO_TEST_CASE( failingMock )
{
	BOOST_REQUIRE_THROW({
			PQ::Mock m ("user=postgres dbname=postgres", "pqmock", { rootDir / "badMock.sql" });
		}, DB::Error);
}

