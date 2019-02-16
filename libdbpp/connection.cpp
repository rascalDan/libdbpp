#include "connection.h"
#include "modifycommand.h"
#include "selectcommand.h"
#include "error.h"
#include <factory.impl.h>
#include <compileTimeFormatter.h>
#include <sqlParse.h>
#include <system_error>

DB::ConnectionError::ConnectionError() :
	FailureTime(time(NULL))
{
}

std::string
DB::TransactionStillOpen::message() const throw()
{
	return "A transaction is still open.";
}

DB::Connection::Connection() :
	txOpenDepth(0)
{
}

DB::Connection::~Connection() = default;

void
DB::Connection::execute(const std::string & sql, const CommandOptionsCPtr & opts)
{
	modify(sql, opts)->execute(true);
}

void
DB::Connection::finish() const
{
	if (inTx()) {
		throw TransactionStillOpen();
	}
}

AdHocFormatter(SavePointFmt, "tx_sp_%?_%?");

void
DB::Connection::beginTx()
{
	if (inTx()) {
		savepoint(SavePointFmt::get(this, txOpenDepth));
	}
	else {
		beginTxInt();
	}
	txOpenDepth += 1;
}

void
DB::Connection::commitTx()
{
	switch (txOpenDepth) {
		case 0:
			throw TransactionRequired();
		case 1:
			commitTxInt();
			break;
		default:
			releaseSavepoint(SavePointFmt::get(this, txOpenDepth - 1));
			break;
	}
	txOpenDepth -= 1;
}

void
DB::Connection::rollbackTx()
{
	switch (txOpenDepth) {
		case 0:
			throw TransactionRequired();
		case 1:
			rollbackTxInt();
			break;
		default:
			rollbackToSavepoint(SavePointFmt::get(this, txOpenDepth - 1));
			break;
	}
	txOpenDepth -= 1;
}

bool
DB::Connection::inTx() const
{
	return txOpenDepth > 0;
}

void
DB::Connection::executeScript(std::istream & f, const std::filesystem::path & s)
{
	DB::SqlExecuteScript p(f, s, this);
	p.Execute();
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

size_t
DB::Connection::bulkUploadData(std::istream & in) const
{
	if (!in.good()) throw std::runtime_error("Input stream is not good");
	char buf[BUFSIZ];
	size_t total = 0;
	for (std::streamsize r; (r = in.readsome(buf, sizeof(buf))) > 0; ) {
		bulkUploadData(buf, r);
		total += r;
	}
	return total;
}

size_t
DB::Connection::bulkUploadData(FILE * in) const
{
	if (!in) throw std::runtime_error("Input file handle is null");
	char buf[BUFSIZ];
	size_t total = 0, r;
	while ((r = fread(buf, 1, sizeof(buf), in)) > 0) {
		bulkUploadData(buf, r);
		total += r;
	}
	if ((int)r < 0) {
		throw std::system_error(-r, std::system_category());
	}
	return total;

}

AdHocFormatter(PluginLibraryFormat, "libdbpp-%?.so");
std::optional<std::string>
DB::Connection::resolvePlugin(const std::type_info &, const std::string_view & name)
{
	return PluginLibraryFormat::get(name);
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

DB::TransactionScope::TransactionScope(DB::Connection & c) :
	conn(c)
{
	conn.beginTx();
}

// It is acceptable for a commit to fail
// NOLINTNEXTLINE(bugprone-exception-escape)
DB::TransactionScope::~TransactionScope()
{
	if (std::uncaught_exceptions()) {
		try {
			conn.rollbackTx();
		}
		catch (...) {
			// Nothing we can do here
		}
	}
	else {
		conn.commitTx();
	}
}

INSTANTIATEFACTORY(DB::Connection, std::string);
PLUGINRESOLVER(DB::ConnectionFactory, DB::Connection::resolvePlugin);

