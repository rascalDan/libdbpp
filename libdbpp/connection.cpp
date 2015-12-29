#include "connection.h"
#include "modifycommand.h"
#include "selectcommand.h"
#include "error.h"
#include <factory.impl.h>
#include <buffer.h>
#include <sqlParse.h>
#include <boost/shared_ptr.hpp>

DB::ConnectionError::ConnectionError() :
	FailureTime(time(NULL))
{
}

std::string
DB::TransactionStillOpen::message() const throw()
{
	return "A transaction is still open.";
}

DB::Connection::~Connection()
{
}

void
DB::Connection::execute(const std::string & sql)
{
	modify(sql)->execute(true);
}


DB::SelectCommandPtr
DB::Connection::select(const std::string & sql)
{
	return DB::SelectCommandPtr(newSelectCommand(sql));
}

DB::ModifyCommandPtr
DB::Connection::modify(const std::string & sql)
{
	return DB::ModifyCommandPtr(newModifyCommand(sql));
}

void
DB::Connection::executeScript(std::istream & f, const boost::filesystem::path & s)
{
	if (!f.good()) {
		throw SqlParseException("Script stream not in good state.", 0);
	}
	DB::SqlParse p(f, s, this);
	while (p.yylex()) ;
}

void
DB::Connection::savepoint(const std::string & sp)
{
	execute("SAVEPOINT " + sp);
}

void
DB::Connection::rollbackToSavepoint(const std::string & sp)
{
	execute("ROLLBACK TO SAVEPOINT " + sp);
}

void
DB::Connection::releaseSavepoint(const std::string & sp)
{
	execute("RELEASE SAVEPOINT " + sp);
}

void
DB::Connection::beginBulkUpload(const char *, const char *)
{
	throw DB::BulkUploadNotSupported();
}

void
DB::Connection::endBulkUpload(const char *)
{
	throw DB::BulkUploadNotSupported();
}

size_t
DB::Connection::bulkUploadData(const char *, size_t) const
{
	throw DB::BulkUploadNotSupported();
}

boost::optional<std::string>
DB::Connection::resolvePlugin(const std::type_info &, const std::string & name)
{
	return stringbf("libdbpp-%s.so", name);
}

int64_t
DB::Connection::insertId()
{
	throw std::runtime_error("insertId not implemented for this driver.");
}

std::string
DB::TransactionRequired::message() const throw()
{
	return "A transaction must be opened before performing this operation";
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

