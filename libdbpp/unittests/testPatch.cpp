#define BOOST_TEST_MODULE DbTablePatch
#include <boost/test/unit_test.hpp>

#include <connection.h>
#include <definedDirs.h>
#include <pq-mock.h>
#include <command.h>
#include <buffer.h>

class Mock : public PQ::Mock {
	public:
		Mock() :
			PQ::Mock("user=postgres dbname=postgres", "pqmock", { rootDir / "patch.sql" })
		{
		}
};

class OrderByA : public DB::SqlWriter {
	public:
		void writeSql(AdHoc::Buffer & b)
		{
			b.append("a");
		}
};

class WhereAequals1 : public DB::SqlWriter {
	public:
		void writeSql(AdHoc::Buffer & b)
		{
			b.append("a.a = ?");
		}
		void bindParams(DB::Command * cmd, unsigned int & o)
		{
			cmd->bindParamI(o++, 1);
		}
};

class MarkDeleted : public DB::SqlWriter {
	public:
		void writeSql(AdHoc::Buffer & b)
		{
			b.append("deleted = ?");
		}
		void bindParams(DB::Command * cmd, unsigned int & o)
		{
			cmd->bindParamB(o++, true);
		}
};

BOOST_FIXTURE_TEST_SUITE(mock, Mock);

BOOST_AUTO_TEST_CASE( sanityFail )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	BOOST_REQUIRE_THROW(db->patchTable(&tp), DB::PatchCheckFailure);
}

BOOST_AUTO_TEST_CASE( noTx )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	BOOST_REQUIRE_THROW(db->patchTable(&tp), DB::TransactionRequired);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE( testBasic )
{
	Mock mock;
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
	db->beginTx();
	auto r2 = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(0, r2.deletes);
	BOOST_REQUIRE_EQUAL(0, r2.inserts);
	BOOST_REQUIRE_EQUAL(0, r2.updates);
}

BOOST_AUTO_TEST_CASE( allKeys )
{
	Mock mock;
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b"};
	tp.pk = {"a", "b"};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(0, r.updates);
}

BOOST_AUTO_TEST_CASE( testOrder )
{
	Mock mock;
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	OrderByA order;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	tp.order = &order;
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

BOOST_AUTO_TEST_CASE( testWhere )
{
	Mock mock;
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	WhereAequals1 where;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	tp.where = &where;
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(0, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

BOOST_AUTO_TEST_CASE( testInstead )
{
	Mock mock;
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	MarkDeleted mark;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	tp.insteadOfDelete = &mark;
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

