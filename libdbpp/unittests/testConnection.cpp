#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include <factory.h>
#include <connection.h>
#include <definedDirs.h>
#include <fstream>
#include <vector>

// LCOV_EXCL_START
class MockDb : public DB::Connection {
	public:
		MockDb(const std::string &) {}

		void	finish() const {}
		int		beginTx() const { return 0; }
		int		commitTx() const { return 0; }
		int		rollbackTx() const { return 0; }
		bool	inTx() const { return false; }
		void	savepoint(const std::string &) const {}
		void	rollbackToSavepoint(const std::string &) const {}
		void	releaseSavepoint(const std::string &) const {}
		void	ping() const {}
		DB::BulkDeleteStyle bulkDeleteStyle() const { return DB::BulkDeleteUsingUsing; }
		DB::BulkUpdateStyle bulkUpdateStyle() const { return DB::BulkUpdateUsingJoin; }

		void execute(const std::string & sql) const {
			executed.push_back(sql);
		}
		DB::SelectCommand * newSelectCommand(const std::string &) const { return nullptr; }
		DB::ModifyCommand * newModifyCommand(const std::string &) const { return nullptr; }

		void beginBulkUpload(const char *, const char *) const {}
		void endBulkUpload(const char *) const {}
		size_t bulkUploadData(const char *, size_t) const {return 0;}

		mutable std::vector<std::string> executed;
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
	BOOST_REQUIRE_THROW(DB::ConnectionFactory::createNew("otherdb", "doesn't matter"), AdHoc::LoadLibraryException);
}

BOOST_AUTO_TEST_CASE( parse )
{
	auto mock = DB::ConnectionFactory::createNew("MockDb", "doesn't matter");
	std::fstream s((rootDir / "parseTest.sql").string());
	BOOST_REQUIRE(s.good());
	mock->executeScript(s, rootDir);
	MockDb * mockdb = dynamic_cast<MockDb *>(mock);
	BOOST_REQUIRE(mockdb);
	BOOST_REQUIRE_EQUAL(2, mockdb->executed.size());
	BOOST_REQUIRE_EQUAL("INSERT INTO name(t, i) VALUES('string', 3)", mockdb->executed[1]);
	delete mock;
}

