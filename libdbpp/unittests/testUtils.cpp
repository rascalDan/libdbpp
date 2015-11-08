#define BOOST_TEST_MODULE DbUtil
#include <boost/test/unit_test.hpp>

#include <factory.h>
#include <connection.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>
#include <definedDirs.h>
#include <fstream>
#include <vector>
#include <mock.h>
#include <boost/date_time/posix_time/posix_time_io.hpp>

class StandardMockDatabase : public PQ::Mock {
	public:
		StandardMockDatabase() : PQ::Mock("user=postgres dbname=postgres", "pqmock", {
				rootDir / "util.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_AUTO_TEST_CASE( forEachRow )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e FROM forEachRow ORDER BY a LIMIT 1"));
	sel->forEachRow<int64_t, double, std::string, boost::posix_time::ptime, boost::posix_time::time_duration>(
			[](auto a, auto b, auto c, auto d, auto e) {
				BOOST_REQUIRE_EQUAL(1, a);
				BOOST_REQUIRE_EQUAL(2.3, b);
				BOOST_REQUIRE_EQUAL("Some text", c);
				BOOST_REQUIRE_EQUAL(boost::posix_time::ptime_from_tm({ 17, 39, 13, 7, 10, 115, 0, 0, 0, 0, 0}), d);
				BOOST_REQUIRE_EQUAL(boost::posix_time::time_duration(4, 3, 2), e);
			});
}

BOOST_AUTO_TEST_CASE( execute )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->execute("UPDATE forEachRow SET a = 2");
}

