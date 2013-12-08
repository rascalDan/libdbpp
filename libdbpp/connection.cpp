#include "connection.h"
#include "modifycommand.h"

DB::Connection::~Connection()
{
}

void
DB::Connection::execute(const std::string & sql) const
{
	ModifyCommand * cmd = newModifyCommand(sql);
	try {
		cmd->execute(true);
		delete cmd;
	}
	catch (...) {
		delete cmd;
		throw;
	}
}

void
DB::Connection::savepoint(const std::string & sp) const
{
	execute("SAVEPOINT " + sp);
}

void
DB::Connection::rollbackToSavepoint(const std::string & sp) const
{
	execute("ROLLBACK TO SAVEPOINT " + sp);
}

void
DB::Connection::releaseSavepoint(const std::string & sp) const
{
	execute("RELEASE SAVEPOINT " + sp);
}

