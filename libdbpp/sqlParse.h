#ifndef DB_SQLPARSE_H
#define DB_SQLPARSE_H

#include <istream>
#include <string>
#include <exception.h>
#include "connection.h"
#include <boost/filesystem/path.hpp>
#ifndef yyFlexLexer
#define yyFlexLexer sqlBaseFlexLexer
#include <FlexLexer.h>
#endif

namespace DB {
	/// Exception representing a failure to parse an SQL script
	class DLL_PUBLIC SqlParseException : public AdHoc::StdException {
		public:
			/// Create a new SqlParseException
			/// @param what What went wrong
			/// @param line What line number
			SqlParseException(const char * what, unsigned int line);

		private:
			std::string message() const throw() override;
			const char * reason;
			const unsigned int line;
	};

	/// @cond
	class DLL_PUBLIC SqlParse : public yyFlexLexer {
		public:
			SqlParse(std::istream &, const boost::filesystem::path &);
			int yylex() override;

			virtual void Comment(const std::string &) const = 0;
			virtual void Statement(const std::string &) const = 0;

		protected:
			void LexerError(const char *) override;

		private:
			const boost::filesystem::path scriptDir;
			std::string comment;
			std::string statement;
	};

	class DLL_PUBLIC SqlExecuteScript : public SqlParse {
		public:
			SqlExecuteScript(std::istream &, const boost::filesystem::path &, Connection *);

			void Comment(const std::string &) const override;
			void Statement(const std::string &) const override;

		private:
			DB::Connection * const conn;
	};
	/// @endcond
}

#endif

