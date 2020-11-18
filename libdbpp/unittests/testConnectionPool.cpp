#define BOOST_TEST_MODULE DbConnectionPool
#include <boost/test/unit_test.hpp>

#include <buffer.h>
#include <connectionPool.h>
#include <pq-mock.h>

class MockPool : public DB::PluginMock<PQ::Mock>, public DB::ConnectionPool {
public:
	MockPool() :
		PluginMock<PQ::Mock>("pqmock", {}, "user=postgres dbname=postgres"), DB::ConnectionPool(4, 2, "postgresql",
																					 stringbf("user=postgres dbname=%s",
																							 databaseName()))
	{
	}
};

BOOST_AUTO_TEST_CASE(basic)
{
	MockPool pool;
	DB::Connection * cr;
	{
		auto c = pool.get();
		c->beginTx();
		c->commitTx();
		cr = c.get();
	}
	{
		auto c = pool.get();
		BOOST_REQUIRE_EQUAL(cr, c.get());
	}
	{
		auto c1 = pool.get();
		auto c2 = pool.get();
		auto c3 = pool.get();
		auto c4 = pool.get();
		BOOST_REQUIRE_EQUAL(4, pool.inUseCount());
	}
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(2, pool.availableCount());
}
