#define BOOST_TEST_MODULE DbUtil
#include <boost/test/unit_test.hpp>

#include <connection.h>
#include <selectcommand.h>
#include <modifycommand.h>
#include <selectcommandUtil.impl.h>
#include <definedDirs.h>
#include <fstream>
#include <pq-mock.h>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <IceUtil/Exception.h>
#include <IceUtil/Optional.h>

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

BOOST_AUTO_TEST_CASE( nullBind )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto ins = db->modify("INSERT INTO forEachRow VALUES(?, ?, ?, ?, ?, ?)");
	ins->bindParamI(0, boost::optional<int>());
	ins->bindParamF(1, boost::optional<double>());
	ins->bindParamS(2, boost::optional<Glib::ustring>());
	ins->bindParamT(3, boost::optional<boost::posix_time::ptime>());
	ins->bindParamT(4, boost::optional<boost::posix_time::time_duration>());
	ins->bindParamB(5, boost::optional<bool>());
	ins->execute();
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e, f FROM forEachRow WHERE a IS NULL AND b IS NULL AND c IS NULL AND d IS NULL AND e IS NULL AND f IS NULL"));
	unsigned int count = 0;
	for (const auto & row : sel->as<>()) {
		(void)row;
		count += 1;
	}
	BOOST_REQUIRE_EQUAL(1, count);
}

BOOST_AUTO_TEST_CASE( iceNullBind )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto ins = db->modify("INSERT INTO forEachRow VALUES(?, ?, ?, ?, ?, ?)");
	ins->bindParamI(0, IceUtil::Optional<int>());
	ins->bindParamF(1, IceUtil::Optional<double>());
	ins->bindParamS(2, IceUtil::Optional<Glib::ustring>());
	ins->bindParamT(3, IceUtil::Optional<boost::posix_time::ptime>());
	ins->bindParamT(4, IceUtil::Optional<boost::posix_time::time_duration>());
	ins->bindParamB(5, IceUtil::Optional<bool>());
	ins->execute();
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b, c, d, e, f FROM forEachRow WHERE a IS NULL AND b IS NULL AND c IS NULL AND d IS NULL AND e IS NULL AND f IS NULL"));
	unsigned int count = 0;
	for (const auto & row : sel->as<>()) {
		(void)row;
		count += 1;
	}
	BOOST_REQUIRE_EQUAL(2, count);
}

BOOST_AUTO_TEST_CASE( charStarBindNull )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->modify("DELETE FROM forEachRow")->execute();
	auto ins = db->modify("INSERT INTO forEachRow(a, c) VALUES(?, ?)");
	char * cs = NULL;
	char * cs2 = strdup("a thing");
	ins->bindParamS(0, cs);
	ins->bindParamS(1, cs2);
	ins->execute();
	const char * ccs = cs;
	const char * ccs2 = cs2;
	ins->bindParamS(0, ccs);
	ins->bindParamS(1, ccs2);
	ins->execute();
	const char * const ccsc = ccs;
	const char * const ccsc2 = ccs2;
	ins->bindParamS(0, ccsc);
	ins->bindParamS(1, ccsc2);
	ins->execute();
	free(cs2);
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, c FROM forEachRow"));
	for (const auto & row : sel->as<boost::optional<int64_t>, boost::optional<std::string>>()) {
		BOOST_REQUIRE(row[0].isNull());
		BOOST_REQUIRE(!row[1].isNull());
	}
}

BOOST_AUTO_TEST_CASE( bindIntPtr )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->modify("DELETE FROM forEachRow")->execute();
	auto ins = db->modify("INSERT INTO forEachRow(a, b) VALUES(?, ?)");
	int * is = NULL;
	int * is2 = new int(53);
	ins->bindParamI(0, is);
	ins->bindParamI(1, is2);
	ins->execute();
	const int * cis = is;
	const int * cis2 = is2;
	ins->bindParamI(0, cis);
	ins->bindParamI(1, cis2);
	ins->execute();
	const int * const cisc = cis;
	const int * const cisc2 = cis2;
	ins->bindParamI(0, cisc);
	ins->bindParamI(1, cisc2);
	ins->execute();
	delete is2;
	auto sel = DB::SelectCommandPtr(db->newSelectCommand("SELECT a, b FROM forEachRow"));
	unsigned int total = 0;
	for (const auto & row : sel->as<boost::optional<int64_t>, boost::optional<double>>()) {
		BOOST_REQUIRE(row[0].isNull());
		BOOST_REQUIRE(!row[1].isNull());
		total += *row.value<1>();
	}
	BOOST_REQUIRE_EQUAL(159, total);
}

