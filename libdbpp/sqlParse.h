#ifndef DB_SQLPARSE_H
#define DB_SQLPARSE_H

#include <istream>
#include <string>
#include "connection.h"
#include <boost/filesystem/path.hpp>
#ifndef yyFlexLexer
#define yyFlexLexer sqlBaseFlexLexer
#include <FlexLexer.h>
#endif

namespace DB {

/// @cond
class SqlParse : public yyFlexLexer {
	public:
		SqlParse(std::istream &, const boost::filesystem::path &, const Connection *);
		int yylex() override;

		void Comment(const std::string &) const;
		void Statement(const std::string &) const;

	protected:
		void LexerError(const char *) override;

	private:
		const DB::Connection * conn;
		const boost::filesystem::path scriptDir;
		std::string comment;
		std::string statement;
};
/// @endcond

}

#endif

