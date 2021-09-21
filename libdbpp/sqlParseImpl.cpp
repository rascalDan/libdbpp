#include "connection.h"
#include "sqlParse.h"
#include <compileTimeFormatter.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace DB {
	SqlParseException::SqlParseException(const char * r, unsigned int l) : reason(r), line(l) { }

	AdHocFormatter(SqlParseExceptionMsg, "Error parsing SQL script: %? at line %?");
	std::string
	SqlParseException::message() const noexcept
	{
		return SqlParseExceptionMsg::get(reason, line);
	}

	SqlParse::SqlParse(std::istream & f, std::filesystem::path s) : yyFlexLexer(&f, nullptr), scriptDir(std::move(s))
	{
		if (!f.good()) {
			throw SqlParseException("Script stream not in good state.", 0);
		}
	}

	void
	SqlParse::Execute()
	{
		while (yylex()) { }
	}

	void
	SqlParse::LexerError(const char * msg)
	{
		throw std::runtime_error(msg);
	}

	SqlExecuteScript::SqlExecuteScript(std::istream & f, const std::filesystem::path & s, DB::Connection * c) :
		SqlParse(f, s), conn(c)
	{
	}

	void
	SqlExecuteScript::Comment(const std::string &) const
	{
	}

	void
	SqlExecuteScript::Statement(const std::string & text) const
	{
		conn->execute(text);
	}

}
