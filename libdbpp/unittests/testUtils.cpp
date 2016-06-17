#define BOOST_TEST_MODULE DbUtil
#include <boost/test/unit_test.hpp>

#include <connection.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>
#include <definedDirs.h>
#include <fstream>
#include <pq-mock.h>
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
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e, f FROM forEachRow ORDER BY a LIMIT 1"));
	sel->forEachRow<int64_t, double, std::string, boost::posix_time::ptime, boost::posix_time::time_duration, bool>(
			[](auto a, auto b, auto c, auto d, auto e, auto f) {
				BOOST_REQUIRE_EQUAL(1, a);
				BOOST_REQUIRE_CLOSE(4.3, b, 0.001);
				BOOST_REQUIRE_EQUAL("Some text", c);
				BOOST_REQUIRE_EQUAL(boost::posix_time::ptime_from_tm({ 17, 39, 13, 7, 10, 115, 0, 0, 0, 0, 0}), d);
				BOOST_REQUIRE_EQUAL(boost::posix_time::time_duration(4, 3, 2), e);
				BOOST_REQUIRE_EQUAL(true, f);
			});
}

BOOST_AUTO_TEST_CASE( forEachRowNulls )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e, f FROM forEachRow ORDER BY a DESC LIMIT 1"));
	sel->forEachRow<int64_t, boost::optional<double>, std::string, boost::optional<boost::posix_time::ptime>, boost::optional<boost::posix_time::time_duration>, bool>(
			[](auto a, auto b, auto c, auto d, auto e, auto f) {
				BOOST_REQUIRE_EQUAL(2, a);
				BOOST_REQUIRE(b);
				BOOST_REQUIRE_CLOSE(4.3, *b, 0.001);
				BOOST_REQUIRE_EQUAL("Some text", c);
				BOOST_REQUIRE(!d);
				BOOST_REQUIRE(!e);
				BOOST_REQUIRE_EQUAL(false, f);
			});
}

BOOST_AUTO_TEST_CASE( stdforOverRows )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	unsigned int count = 0;
	int64_t totalOfa = 0;
	std::string totalOfc;
	auto sel = db->select("SELECT a, c FROM forEachRow ORDER BY a DESC");
	for (const auto & row : sel->as<int64_t, std::string>()) {
		count += 1;
		BOOST_REQUIRE_EQUAL("a", row[0].name);
		BOOST_REQUIRE_EQUAL(1, row["c"].colNo);
		int64_t a = row.value<0>();
		totalOfa += a;
		totalOfc += row.value<1>();
	}
	BOOST_REQUIRE_EQUAL(count, 2);
	BOOST_REQUIRE_EQUAL(totalOfa, 3);
	BOOST_REQUIRE_EQUAL(totalOfc, "Some textSome text");
}

BOOST_AUTO_TEST_CASE( execute )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->execute("UPDATE forEachRow SET a = 2");
}

BOOST_AUTO_TEST_CASE( columns )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e FROM forEachRow ORDER BY a LIMIT 1"));
	sel->execute();
	BOOST_REQUIRE_THROW((*sel)[5], DB::ColumnIndexOutOfRange);
	BOOST_REQUIRE_THROW((*sel)[-1], DB::ColumnIndexOutOfRange);
	BOOST_REQUIRE_EQUAL(0, (*sel)[0].colNo);
	BOOST_REQUIRE_EQUAL(4, (*sel)[4].colNo);
	BOOST_REQUIRE_EQUAL("c", (*sel)[2].name);
	BOOST_REQUIRE_EQUAL(0, (*sel)["a"].colNo);
	BOOST_REQUIRE_EQUAL(4, (*sel)["e"].colNo);
	BOOST_REQUIRE_EQUAL(5, sel->columnCount());
	BOOST_REQUIRE_EQUAL(1, sel->getOrdinal("b"));
	BOOST_REQUIRE_THROW((*sel)["f"], DB::ColumnDoesNotExist);
	BOOST_REQUIRE_THROW((*sel)["aa"], DB::ColumnDoesNotExist);
	BOOST_REQUIRE_THROW((*sel)[""], DB::ColumnDoesNotExist);
}

BOOST_AUTO_TEST_CASE( extract )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c FROM forEachRow WHERE f"));
	BOOST_REQUIRE(sel->fetch());
	int64_t lint;
	double ldouble;
	std::string lstring;
	// Plain
	(*sel)[0] >> lint;
	(*sel)[1] >> ldouble;
	(*sel)[2] >> lstring;
	BOOST_REQUIRE_EQUAL(2, lint);
	BOOST_REQUIRE_CLOSE(4.3, ldouble, 0.001);
	BOOST_REQUIRE_EQUAL("Some text", lstring);
	// Converted
	(*sel)[1] >> lint;
	(*sel)[0] >> ldouble;
	BOOST_REQUIRE_EQUAL(4, lint);
	BOOST_REQUIRE_CLOSE(2, ldouble, 0.001);
	// Bad conversions
	BOOST_REQUIRE_THROW((*sel)[2] >> lint, DB::InvalidConversion);
	BOOST_REQUIRE_THROW((*sel)[2] >> ldouble, DB::InvalidConversion);
	BOOST_REQUIRE_THROW((*sel)[1] >> lstring, DB::InvalidConversion);
	BOOST_REQUIRE_THROW((*sel)[0] >> lstring, DB::InvalidConversion);
	BOOST_REQUIRE(!sel->fetch());
}

BOOST_AUTO_TEST_CASE( bulkLoadStream )
{
	std::ifstream in((rootDir / "source.dat").string());
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->beginBulkUpload("bulk1", "");
	BOOST_REQUIRE_EQUAL(56, db->bulkUploadData(in));
	db->endBulkUpload(nullptr);
	db->select("SELECT COUNT(*) FROM bulk1")->forEachRow<int64_t>([](auto n) {
			BOOST_REQUIRE_EQUAL(4, n);
		});
}

BOOST_AUTO_TEST_CASE( bulkLoadFile )
{
	auto f = fopen((rootDir / "source.dat").c_str(), "r");
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->beginBulkUpload("bulk2", "");
	BOOST_REQUIRE_EQUAL(56, db->bulkUploadData(f));
	db->endBulkUpload(nullptr);
	fclose(f);
	db->select("SELECT COUNT(*) FROM bulk2")->forEachRow<int64_t>([](auto n) {
			BOOST_REQUIRE_EQUAL(4, n);
		});
}

