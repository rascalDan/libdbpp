#include "command.h"

DB::Command::Command(const std::string & s) :
	sql(s)
{
}

DB::Command::~Command()
{
}

DB::ParameterTypeNotSupported::ParameterTypeNotSupported()
{
}

DB::ParameterOutOfRange::ParameterOutOfRange()
{
}

DB::CommandOptions::CommandOptions(std::size_t h) :
	hash(h)
{
}

void
DB::Command::bindParamS(unsigned int i, const char * const o)
{
	if (o)
		bindParamS(i, Glib::ustring(o));
	else
		bindNull(i);
}


void
DB::Command::bindParamS(unsigned int i, char * const o)
{
	if (o)
		bindParamS(i, Glib::ustring(o));
	else
		bindNull(i);
}

