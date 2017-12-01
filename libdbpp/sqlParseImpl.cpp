#include <stdexcept>
#include <compileTimeFormatter.h>
#include "sqlParse.h"

namespace DB {
  SqlParseException::SqlParseException(const char * r, unsigned int l) : reason(r), line(l) { }

	AdHocFormatter(SqlParseExceptionMsg, "Error parsing SQL script: %? at line %?");
  std::string
  SqlParseException::message() const throw()
  {
    return SqlParseExceptionMsg::get(reason, line);
  }

  SqlParse::SqlParse(std::istream & f, const boost::filesystem::path & s) :
    yyFlexLexer(&f, NULL),
    scriptDir(s)
  {
    if (!f.good()) {
      throw SqlParseException("Script stream not in good state.", 0);
    }
  }

  void
  SqlParse::Execute()
  {
    while (yylex()) ;
  }

  void
  SqlParse::LexerError(const char * msg)
  {
    throw std::runtime_error(msg);
  }

  SqlExecuteScript::SqlExecuteScript(std::istream & f, const boost::filesystem::path & s, DB::Connection * c) :
    SqlParse(f, s),
    conn(c)
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
