#include "selectcommand.h"

DB::SelectCommand::SelectCommand(const std::string & sql) :
	DB::Command(sql)
{
}

DB::SelectCommand::~SelectCommand()
{
}

