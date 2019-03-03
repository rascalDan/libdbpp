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
#include <testCore.h>

class StandardMockDatabase : public DB::PluginMock<PQ::Mock> {
	public:
		StandardMockDatabase() : DB::PluginMock<PQ::Mock>("pqmock", {
				rootDir / "util.sql" }, "user=postgres dbname=postgres")
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_AUTO_TEST_CASE( forEachRow )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	auto sel = db->select("SELECT a, b, c, d, e, f FROM forEachRow ORDER BY a LIMIT 1");
	sel->forEachRow<int64_t, double, std::string, boost::posix_time::ptime, boost::posix_time::time_duration, bool>(
			[](auto a, auto b, auto c, auto d, auto e, auto f) {
				BOOST_REQUIRE_EQUAL(1, a);
				BOOST_REQUIRE_CLOSE(4.3, b, 0.001);
				BOOST_REQUIRE_EQUAL("Some text", c);
				BOOST_REQUIRE_EQUAL(boost::posix_time::ptime_from_tm({ 17, 39, 13, 7, 10, 115, 0, 0, 0, 0, nullptr}), d);
				BOOST_REQUIRE_EQUAL(boost::posix_time::time_duration(4, 3, 2), e);
				BOOST_REQUIRE_EQUAL(true, f);
			});
}

BOOST_AUTO_TEST_CASE( forEachRowNulls )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	auto sel = db->select("SELECT a, b, c, d, e, f FROM forEachRow ORDER BY a DESC LIMIT 1");
	sel->forEachRow<int64_t, std::optional<double>, std::string, std::optional<boost::posix_time::ptime>, std::optional<boost::posix_time::time_duration>, bool>(
			[](auto && a, auto b, auto c, auto d, auto && e, auto f) {
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
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	unsigned int count = 0;
	int64_t totalOfa = 0;
	std::string totalOfc;
	auto sel = db->select("SELECT a, c FROM forEachRow ORDER BY a DESC");
	for (const auto & row : sel->as<int64_t, std::string>()) {
		count += 1;
		BOOST_REQUIRE_EQUAL("a", row[0].name);
		BOOST_REQUIRE_EQUAL(1, row["c"].colNo);
		// Test old function
		int64_t a = row.value<0>();
		totalOfa += a;
		totalOfc += row.get<1>();
	}
	BOOST_REQUIRE_EQUAL(count, 2);
	BOOST_REQUIRE_EQUAL(totalOfa, 3);
	BOOST_REQUIRE_EQUAL(totalOfc, "Some textSome text");
}

BOOST_AUTO_TEST_CASE( stdforOverRowsStructuredBinding )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	unsigned int count = 0;
	int64_t totalOfa = 0;
	std::string totalOfc;
	auto sel = db->select("SELECT a, c FROM forEachRow ORDER BY a DESC");
	for (const auto [ a, c ] : sel->as<int64_t, std::string_view>()) {
		count += 1;
		totalOfa += a;
		totalOfc += c;
	}
	BOOST_REQUIRE_EQUAL(count, 2);
	BOOST_REQUIRE_EQUAL(totalOfa, 3);
	BOOST_REQUIRE_EQUAL(totalOfc, "Some textSome text");
}

BOOST_AUTO_TEST_CASE( stdforOverRowsStructuredBindingOptional )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	unsigned int count = 0;
	int64_t totalOfa = 0;
	std::string totalOfc;
	auto sel = db->select("SELECT a, c FROM forEachRow ORDER BY a DESC");
	for (const auto [ a, c ] : sel->as<std::optional<int64_t>, std::optional<std::string>>()) {
		count += 1;
		BOOST_REQUIRE(a);
		totalOfa += *a;
		BOOST_REQUIRE(c);
		totalOfc += *c;
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
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	auto sel = db->select("SELECT a, b, c, d, e FROM forEachRow ORDER BY a LIMIT 1");
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
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	auto sel = db->select("SELECT a, b, c FROM forEachRow WHERE f");
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
	std::ifstream in(rootDir / "source.dat");
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

using StringTypes = boost::mpl::list<std::string, std::string_view, Glib::ustring>;
BOOST_AUTO_TEST_CASE_TEMPLATE( nullBind, Str, StringTypes )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	db->execute("DELETE FROM forEachRow");
	auto ins = db->modify("INSERT INTO forEachRow VALUES(?, ?, ?, ?, ?, ?)");
	ins->bindParamI(0, std::optional<int>());
	ins->bindParamF(1, std::optional<double>());
	ins->bindParamS(2, std::optional<Glib::ustring>());
	ins->bindParamT(3, std::optional<boost::posix_time::ptime>());
	ins->bindParamT(4, std::optional<boost::posix_time::time_duration>());
	ins->bindParamB(5, std::optional<bool>());
	ins->execute();
	auto sel = db->select("SELECT a, b, c, d, e, f FROM forEachRow WHERE a IS NULL AND b IS NULL AND c IS NULL AND d IS NULL AND e IS NULL AND f IS NULL");
	unsigned int count = 0;
	for (const auto & row : sel->as<int, std::optional<double>, Str>()) {
		count += 1;
		BOOST_CHECK_THROW(row.template get<0>(), DB::UnexpectedNullValue);
		auto nd = row.template get<1>();
		BOOST_CHECK(!nd.has_value());
		BOOST_CHECK_THROW(row.template get<2>(), DB::UnexpectedNullValue);
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
	auto sel = db->select("SELECT a, b, c, d, e, f FROM forEachRow WHERE a IS NULL AND b IS NULL AND c IS NULL AND d IS NULL AND e IS NULL AND f IS NULL");
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
	char * cs = nullptr;
	std::string cs2("a thing");
	ins->bindParamS(0, cs);
	ins->bindParamS(1, cs2.c_str());
	ins->execute();
	const char * ccs = cs;
	const char * ccs2 = cs2.c_str();
	ins->bindParamS(0, ccs);
	ins->bindParamS(1, ccs2);
	ins->execute();
	const char * const ccsc = ccs;
	const char * const ccsc2 = ccs2;
	ins->bindParamS(0, ccsc);
	ins->bindParamS(1, ccsc2);
	ins->execute();
	auto sel = db->select("SELECT a, c FROM forEachRow");
	for (const auto & row : sel->as<std::optional<int64_t>, std::optional<std::string>>()) {
		BOOST_REQUIRE(row[0].isNull());
		BOOST_REQUIRE(!row[1].isNull());
	}
}

BOOST_AUTO_TEST_CASE( bind )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto m = db->modify("doesn't matter, only testing compile");
	m->bindParamI(0, (unsigned char)1);
	m->bindParamI(0, (char)1);
	m->bindParamI(0, (time_t)1);
}

BOOST_AUTO_TEST_CASE( bindIntPtr )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	db->modify("DELETE FROM forEachRow")->execute();
	auto ins = db->modify("INSERT INTO forEachRow(a, b) VALUES(?, ?)");
	int * is = nullptr;
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
	auto sel = db->select("SELECT a, b FROM forEachRow");
	double total = 0;
	for (const auto & row : sel->as<std::optional<int64_t>, std::optional<double>>()) {
		BOOST_REQUIRE(row[0].isNull());
		BOOST_REQUIRE(!row[1].isNull());
		total += *row.value<1>();
	}
	BOOST_REQUIRE_EQUAL(159, total);
}

BOOST_FIXTURE_TEST_CASE( traits_bind, DB::TestCore )
{
	auto db = DB::ConnectionPtr(DB::MockDatabase::openConnectionTo("pqmock"));
	auto cmd = db->select("select x is null, format('%s', x) from (select ? x) d");
	auto check = [cmd](int line, const auto & v, bool isNull, const auto & strval) {
		BOOST_TEST_CONTEXT("line " << line) {
			cmd->bindParam(0, v);
			for (const auto & [null, str] : cmd->as<bool, std::string_view>()) {
				BOOST_CHECK_EQUAL(null, isNull);
				BOOST_CHECK_EQUAL(str, strval);
			}
		}
	};
	check(__LINE__, std::nullopt, true, "");
	check(__LINE__, nullptr, true, "");
	check(__LINE__, testInt, false, "43");
	check(__LINE__, std::make_unique<short>(testInt), false, "43");
	check(__LINE__, std::unique_ptr<short>(), true, "");
	check(__LINE__, (unsigned int)testInt, false, "43");
	check(__LINE__, &testInt, false, "43");
	check(__LINE__, testDouble, false, "3.14");
	check(__LINE__, std::make_shared<float>(testDouble), false, "3.14");
	check(__LINE__, std::shared_ptr<float>(), true, "");
	check(__LINE__, (float)testDouble, false, "3.14");
	check(__LINE__, &testDouble, false, "3.14");
	check(__LINE__, testString, false, testString);
	check(__LINE__, &testString, false, testString);
	check(__LINE__, "str", false, "str");
	check(__LINE__, "", false, "");
	const char * const nullstr = nullptr;
	check(__LINE__, nullstr, true, "");
	const char * const str = "str";
	check(__LINE__, str, false, "str");
	check(__LINE__, std::string(), false, "");
	check(__LINE__, std::string_view(), false, "");
	check(__LINE__, Glib::ustring(), false, "");
	check(__LINE__, Glib::ustring("foo"), false, "foo");
	check(__LINE__, testDateTime, false, "2015-05-02T01:36:33");
	check(__LINE__, testInterval, false, "01:02:03");
	check(__LINE__, &testInterval, false, "01:02:03");
	decltype(testInterval) * nullInterval = nullptr;
	check(__LINE__, nullInterval, true, "");
	check(__LINE__, true, false, "1");
	check(__LINE__, false, false, "0");
	check(__LINE__, std::optional<int>(4), false, "4");
	check(__LINE__, std::optional<int>(), true, "");
}

BOOST_AUTO_TEST_CASE( testBlobRaw )
{
	DB::Blob ptr(this, 1);
	BOOST_REQUIRE_EQUAL(ptr.data, this);
	BOOST_REQUIRE_EQUAL(ptr.len, 1);
}

BOOST_AUTO_TEST_CASE( testBlobObject )
{
	int32_t x = 20;
	DB::Blob obj(&x);
	BOOST_REQUIRE_EQUAL(obj.data, &x);
	BOOST_REQUIRE_EQUAL(obj.len, 4);
}

BOOST_AUTO_TEST_CASE( testBlobVec )
{
	std::vector<uint8_t> buf(20, 0);
	DB::Blob vec(buf);
	BOOST_REQUIRE_EQUAL(vec.data, &buf[0]);
	BOOST_REQUIRE_EQUAL(vec.len, 20);
}

struct S {
	int64_t a;
	int64_t b;
};
BOOST_STATIC_ASSERT(sizeof(S) == 16);

BOOST_AUTO_TEST_CASE( testBlobStruct )
{
	S s = { 8, 4 };
	DB::Blob str(&s);
	BOOST_REQUIRE_EQUAL(str.data, &s);
	BOOST_REQUIRE_EQUAL(str.len, 16);
}

BOOST_AUTO_TEST_CASE( testBlobVecStruct )
{
	std::vector<S> buf(20, {4, 8});
	DB::Blob vec(buf);
	BOOST_REQUIRE_EQUAL(vec.data, &buf[0]);
	BOOST_REQUIRE_EQUAL(vec.len, 20 * 16);
}

BOOST_AUTO_TEST_CASE( testBlobCompare )
{
	std::vector<S> buf1(20, {4, 8});
	DB::Blob vec1(buf1);

	// NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
	std::vector<S> buf2(buf1);
	DB::Blob vec2(buf2);

	std::vector<S> buf3(buf2);
	buf3.pop_back();
	DB::Blob vec3(buf3);

	BOOST_REQUIRE_EQUAL(vec1, vec2);
	BOOST_REQUIRE(!(vec1 == vec3));
	BOOST_REQUIRE(!(vec2 == vec3));
}

// These just compile time support, actual data extraction should be tested by the implementing connector.
template<typename T>
void
testExtractT(DB::Row<T> row) {
	row.template value<0>();
}

template<typename T>
void
testExtractT(const DB::SelectCommandPtr & sel) {
	// test default construct
	T test;
	(void)test;
	for (const auto & row : sel->as<T>()) { testExtractT(row); }
#ifdef __clang__
	// Clang cannot compile this for reasons largely todo with ambiguousness in the spec
	// Fixed when we move to std::chrono
	// NOLINTNEXTLINE(bugprone-suspicious-semicolon,hicpp-braces-around-statements)
	if constexpr (!std::is_same<T, boost::posix_time::time_duration>::value) {
#else
	if constexpr (true) {
#endif
		for (const auto & row : sel->as<std::optional<T>>()) { testExtractT(row); }
	}
}

BOOST_AUTO_TEST_CASE( testExtractTypes )
{
	auto db = DB::MockDatabase::openConnectionTo("pqmock");
	auto sel = db->select("SELECT 1 FROM forEachRow LIMIT 0");
	testExtractT<int8_t>(sel);
	testExtractT<int16_t>(sel);
	testExtractT<int32_t>(sel);
	testExtractT<int64_t>(sel);
	testExtractT<bool>(sel);
	testExtractT<std::string>(sel);
	testExtractT<std::string_view>(sel);
	testExtractT<Glib::ustring>(sel);
	testExtractT<float>(sel);
	testExtractT<double>(sel);
	testExtractT<boost::posix_time::ptime>(sel);
	testExtractT<boost::posix_time::time_duration>(sel);
	testExtractT<DB::Blob>(sel);
}

