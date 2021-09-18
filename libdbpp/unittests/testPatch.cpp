#define BOOST_TEST_MODULE DbTablePatch
#include <boost/test/unit_test.hpp>

#include <buffer.h>
#include <command.h>
#include <connection.h>
#include <definedDirs.h>
#include <pq-mock.h>
#include <selectcommandUtil.impl.h>
#include <sqlWriter.h>
#include <tablepatch.h>

class Mock : public DB::PluginMock<PQ::Mock> {
public:
	Mock() : DB::PluginMock<PQ::Mock>("pqmock", {rootDir / "patch.sql"}, "user=postgres dbname=postgres") { }
};

class OrderByA : public DB::StaticSqlWriter {
public:
	OrderByA() : DB::StaticSqlWriter("a") { }
};

class WhereAequals1 : public DB::SqlWriter {
public:
	void
	writeSql(AdHoc::Buffer & b) override
	{
		b.append("a.a = ?");
	}
	void
	bindParams(DB::Command * cmd, unsigned int & o) override
	{
		cmd->bindParamI(o++, 1);
	}
};

class MarkDeleted : public DB::SqlWriter {
public:
	void
	writeSql(AdHoc::Buffer & b) override
	{
		b.append("deleted = ?");
	}
	void
	bindParams(DB::Command * cmd, unsigned int & o) override
	{
		cmd->bindParamB(o++, true);
	}
};

BOOST_FIXTURE_TEST_SUITE(mock, Mock)

BOOST_AUTO_TEST_CASE(sanityFail)
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	BOOST_REQUIRE_THROW(db->patchTable(&tp), DB::PatchCheckFailure);
}

BOOST_AUTO_TEST_CASE(noTx)
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	BOOST_REQUIRE_THROW(db->patchTable(&tp), DB::TransactionRequired);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(testBasic)
{
	Mock mock;
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
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

BOOST_AUTO_TEST_CASE(allKeys)
{
	Mock mock;
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
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

BOOST_AUTO_TEST_CASE(testOrder)
{
	Mock mock;
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	OrderByA order;
	tp.src = "source";
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	tp.order = &order;
	tp.beforeDelete = [](const DB::SelectCommandPtr & i) {
		i->forEachRow<int64_t, int64_t, std::string, std::string>([](auto a, auto b, auto c, auto d) {
			fprintf(stderr, "<< %ld %ld %s %s\n", a, b, c.c_str(), d.c_str());
		});
	};
	tp.beforeUpdate = [](const DB::SelectCommandPtr & i) {
		i->forEachRow<int64_t, int64_t, std::string, std::string, std::string, std::string>(
				[](auto a, auto b, auto c1, auto d1, auto c2, auto d2) {
					fprintf(stderr, "== %ld %ld %s->%s %s->%s\n", a, b, c1.c_str(), c2.c_str(), d1.c_str(), d2.c_str());
				});
	};
	tp.beforeInsert = [](const DB::SelectCommandPtr & i) {
		i->forEachRow<int64_t, int64_t, std::string, std::string>([](auto a, auto b, auto c, auto d) {
			fprintf(stderr, ">> %ld %ld %s %s\n", a, b, c.c_str(), d.c_str());
		});
	};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

BOOST_AUTO_TEST_CASE(testWhere)
{
	Mock mock;
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
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

BOOST_AUTO_TEST_CASE(testInstead)
{
	Mock mock;
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
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

BOOST_AUTO_TEST_CASE(testSrcExprTable)
{
	Mock mock;
	DB::StaticSqlWriter s("source");
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.srcExpr = &s;
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

BOOST_AUTO_TEST_CASE(testSrcExprSelectTable)
{
	Mock mock;
	DB::StaticSqlWriter s("(SELECT * FROM source)");
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.srcExpr = &s;
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(2, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}

class BindInt : public DB::StaticSqlWriter {
public:
	BindInt(const std::string & s, int i) : DB::StaticSqlWriter(s), myInt(i) { }

	void
	bindParams(DB::Command * c, unsigned int & offset) override
	{
		c->bindParamI(offset++, myInt);
	}

	int myInt;
};

BOOST_AUTO_TEST_CASE(testSrcExprSelectFilteredTable)
{
	Mock mock;
	BindInt s("(SELECT s.* FROM source s WHERE s.a = ?)", 1);
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	BOOST_REQUIRE(db);
	DB::TablePatch tp;
	tp.srcExpr = &s;
	tp.dest = "target";
	tp.cols = {"a", "b", "c", "d"};
	tp.pk = {"a", "b"};
	db->beginTx();
	auto r = db->patchTable(&tp);
	db->commitTx();
	BOOST_REQUIRE_EQUAL(2, r.deletes);
	BOOST_REQUIRE_EQUAL(1, r.inserts);
	BOOST_REQUIRE_EQUAL(1, r.updates);
}
