#include "connection.h"
#include "modifycommand.h"
#include "error.h"
#include <factory.impl.h>
#include <buffer.h>
#include <sqlParse.h>
#include <boost/shared_ptr.hpp>

DB::Connection::~Connection()
{
}

void
DB::Connection::execute(const std::string & sql) const
{
	auto cmd = ModifyCommandPtr(newModifyCommand(sql));
	cmd->execute(true);
}

void
DB::Connection::executeScript(std::istream & f, const boost::filesystem::path & s) const
{
	if (!f.good()) {
		throw SqlParseException("Script stream not in good state.", 0);
	}
	DB::SqlParse p(f, s, this);
	while (p.yylex()) ;
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

boost::optional<std::string>
DB::Connection::resolvePlugin(const std::type_info &, const std::string & name)
{
	return stringbf("libdbpp-%s.so", name);
}

int64_t
DB::Connection::insertId() const
{
	throw std::runtime_error("insertId not implemented for this driver.");
}

DB::TablePatch::TablePatch() :
	insteadOfDelete(nullptr),
	where(nullptr),
	order(nullptr)
{
}

void
DB::SqlWriter::bindParams(DB::Command *, unsigned int &)
{
}

DB::TransactionRequired::TransactionRequired() :
	std::logic_error("A transaction must be opened before performing this operation")
{
}

DB::TransactionScope::TransactionScope(DB::Connection * c) :
	conn(c)
{
	conn->beginTx();
}

DB::TransactionScope::~TransactionScope()
{
	if (std::uncaught_exception()) {
		conn->rollbackTx();
	}
	else {
		conn->commitTx();
	}
}

INSTANTIATEFACTORY(DB::Connection, std::string);
PLUGINRESOLVER(DB::ConnectionFactory, DB::Connection::resolvePlugin);

