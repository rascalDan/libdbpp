#include "modifycommand.h"

DB::ModifyCommand::ModifyCommand(const std::string & s) :
	DB::Command(s)
{
}

DB::ModifyCommand::~ModifyCommand()
{
}

DB::NoRowsAffected::NoRowsAffected()
{
}

