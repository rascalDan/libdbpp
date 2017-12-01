#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include <factory.h>
#include <connection.h>
#include <definedDirs.h>
#include <fstream>
#include <vector>
#include <error.h>
#include <sqlParse.h>

typedef std::vector<std::string> SQLs;
BOOST_TEST_SPECIALIZED_COLLECTION_COMPARE(SQLs);

class RecordingParser : std::fstream, public DB::SqlParse {
	public:
		RecordingParser(const boost::filesystem::path & p) :
			std::fstream(p.string()),
			DB::SqlParse(*this, p.parent_path())
		{
		}

    void Comment(const std::string & c) const
		{
			comments.push_back(c);
		}

    void Statement(const std::string & s) const
		{
			executed.push_back(s);
		}

		mutable SQLs comments;
		mutable SQLs executed;
};

template<typename E = DB::SqlParseException>
void assertFail(const boost::filesystem::path & p)
{
	BOOST_TEST_CONTEXT(p) {
		BOOST_REQUIRE_THROW({
			RecordingParser s(p);
			s.Execute();
		}, E);
	}
}

BOOST_AUTO_TEST_CASE( parseBad )
{
	assertFail("/bad");
}

BOOST_AUTO_TEST_CASE( parse )
{
	RecordingParser p(rootDir / "parseTest.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(p.executed.size(), 3);
	BOOST_REQUIRE_EQUAL(p.executed[1], "INSERT INTO name(t, i) VALUES('string', 3)");
}

BOOST_AUTO_TEST_CASE( parseDollarQuote )
{
	RecordingParser p(rootDir / "dollarQuote.sql");
	p.Execute();
}

BOOST_AUTO_TEST_CASE( parseScriptDir )
{
	RecordingParser p(rootDir / "scriptDir.sql");
	p.Execute();
}

BOOST_AUTO_TEST_CASE( parseStringParse )
{
	RecordingParser p(rootDir / "stringParse.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(2, p.executed.size());
	BOOST_REQUIRE_EQUAL("INSERT INTO name(t, i) VALUES('fancy string '' \\' \\r \\n', 7)", p.executed[1]);
}

BOOST_AUTO_TEST_CASE( parseUnterminateComment )
{
	assertFail(rootDir / "unterminatedComment.sql");
}

BOOST_AUTO_TEST_CASE( parseUnterminateDollarQuote )
{
	assertFail(rootDir / "unterminatedDollarQuote.sql");
}

BOOST_AUTO_TEST_CASE( parseUnterminateString )
{
	assertFail(rootDir / "unterminatedString.sql");
}

