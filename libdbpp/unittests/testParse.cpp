#define BOOST_TEST_MODULE DbConnection
#include <boost/test/unit_test.hpp>

#include <connection.h>
#include <definedDirs.h>
#include <error.h>
#include <factory.h>
#include <fstream>
#include <sqlParse.h>
#include <vector>

using SQLs = std::vector<std::string>;
BOOST_TEST_SPECIALIZED_COLLECTION_COMPARE(SQLs);

class RecordingParser : std::fstream, public DB::SqlParse {
public:
	explicit RecordingParser(const std::filesystem::path & p) :
		std::fstream(p), DB::SqlParse(*this, p.parent_path()) { }

	void
	Comment(const std::string & c) const override
	{
		comments.push_back(c);
	}

	void
	Statement(const std::string & s) const override
	{
		executed.push_back(s);
	}

	mutable SQLs comments;
	mutable SQLs executed;
};

template<typename E = DB::SqlParseException>
void
assertFail(const std::filesystem::path & p)
{
	BOOST_TEST_CONTEXT(p) {
		BOOST_REQUIRE_THROW(
				{
					RecordingParser s(p);
					s.Execute();
				},
				E);
	}
}

BOOST_AUTO_TEST_CASE(parseBad)
{
	assertFail("/bad");
}

BOOST_AUTO_TEST_CASE(parse)
{
	RecordingParser p(rootDir / "parseTest.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(p.executed.size(), 3);
	BOOST_REQUIRE_EQUAL(p.executed[1], "INSERT INTO name(t, i) VALUES('string', 3)");
	auto cs = {
			"Single line comment",
			"",
			"",
			"",
			"",
			"Comment",
			"",
			"",
			"",
			"Multi line\n\t comment",
			"! Stupid MySQL terminates",
			"! comments with a ;",
			"! Because reasons",
	};
	BOOST_CHECK_EQUAL_COLLECTIONS(p.comments.begin(), p.comments.end(), cs.begin(), cs.end());
}

BOOST_AUTO_TEST_CASE(parseDollarQuote)
{
	RecordingParser p(rootDir / "dollarQuote.sql");
	p.Execute();
}

BOOST_AUTO_TEST_CASE(parseScriptDir)
{
	RecordingParser p(rootDir / "scriptDir.sql");
	p.Execute();
}

BOOST_AUTO_TEST_CASE(parseStringParse)
{
	RecordingParser p(rootDir / "stringParse.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(2, p.executed.size());
	BOOST_REQUIRE_EQUAL("INSERT INTO name(t, i) VALUES('fancy string '' \\' \\r \\n', 7)", p.executed[1]);
}

BOOST_AUTO_TEST_CASE(indentedStatement)
{
	RecordingParser p(rootDir / "indentedStatement.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(1, p.executed.size());
	BOOST_REQUIRE_EQUAL("SELECT 1", p.executed[0]);
	BOOST_REQUIRE(p.comments.empty());
}

BOOST_AUTO_TEST_CASE(indentedOneLineComment)
{
	RecordingParser p(rootDir / "indentedOneLineComment.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(1, p.comments.size());
	BOOST_REQUIRE_EQUAL("Some comment text", p.comments[0]);
	BOOST_REQUIRE(p.executed.empty());
}

BOOST_AUTO_TEST_CASE(indentedBlockComment)
{
	RecordingParser p(rootDir / "indentedBlockComment.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(1, p.comments.size());
	BOOST_REQUIRE_EQUAL("Some comment text", p.comments[0]);
	BOOST_REQUIRE(p.executed.empty());
}

BOOST_AUTO_TEST_CASE(commentsMixedIn)
{
	RecordingParser p(rootDir / "commentsMixedIn.sql");
	p.Execute();
	BOOST_REQUIRE_EQUAL(1, p.executed.size());
	BOOST_REQUIRE_EQUAL("CREATE TABLE foo(\n\t\tid int,\n\t\ttimestamp time stamp\n\t\t)", p.executed[0]);
	BOOST_REQUIRE_EQUAL(3, p.comments.size());
	BOOST_REQUIRE_EQUAL("Foo contains test things", p.comments[0]);
	BOOST_REQUIRE_EQUAL("Every table deserves an Id, right?", p.comments[1]);
	BOOST_REQUIRE_EQUAL("And a timestamp", p.comments[2]);
}

BOOST_AUTO_TEST_CASE(parseUnterminateComment)
{
	assertFail(rootDir / "unterminatedComment.sql");
}

BOOST_AUTO_TEST_CASE(parseUnterminateDollarQuote)
{
	assertFail(rootDir / "unterminatedDollarQuote.sql");
}

BOOST_AUTO_TEST_CASE(parseUnterminateString)
{
	assertFail(rootDir / "unterminatedString.sql");
}
