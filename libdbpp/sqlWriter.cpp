#include "sqlWriter.h"

void
DB::SqlWriter::bindParams(DB::Command *, unsigned int &)
{
}

DB::StaticSqlWriter::StaticSqlWriter(std::string s) : sql(std::move(s)) { }

void
DB::StaticSqlWriter::writeSql(AdHoc::Buffer & buf)
{
	buf.append(sql);
}
