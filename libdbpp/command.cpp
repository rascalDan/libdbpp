#include "command.h"
#include "connection.h"
#include <factory.impl.h>

INSTANTIATEFACTORY(DB::CommandOptions, std::size_t, const DB::CommandOptionsMap &);
NAMEDFACTORY("", DB::CommandOptions, DB::CommandOptionsFactory);
PLUGINRESOLVER(DB::CommandOptionsFactory, DB::Connection::resolvePlugin);

DB::Command::Command(const std::string & s) :
	sql(s)
{
}

DB::Command::~Command() = default;

DB::ParameterTypeNotSupported::ParameterTypeNotSupported()
{
}

DB::ParameterOutOfRange::ParameterOutOfRange()
{
}

DB::CommandOptions::CommandOptions(std::size_t h, const CommandOptionsMap &) :
	hash(h)
{
}

bool
DB::CommandOptions::isSet(const CommandOptionsMap & map, const std::string & key)
{
	return (map.find(key) != map.end());
}

void
DB::Command::bindParamS(unsigned int i, const char * const o)
{
	if (o)
		bindParamS(i, std::string_view(o));
	else
		bindNull(i);
}


void
DB::Command::bindParamS(unsigned int i, char * const o)
{
	if (o)
		bindParamS(i, std::string_view(o));
	else
		bindNull(i);
}

void
DB::Command::bindParamBLOB(unsigned int, const Blob &)
{
	throw ParameterTypeNotSupported();
}

