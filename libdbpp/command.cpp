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

