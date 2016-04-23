#include "sqlWriter.h"

void
DB::SqlWriter::bindParams(DB::Command *, unsigned int &)
{
}

DB::StaticSqlWriter::StaticSqlWriter(const std::string & s) :
	sql(s)
{
}

void
DB::StaticSqlWriter::writeSql(AdHoc::Buffer & buf)
{
	buf.append(sql);
}

