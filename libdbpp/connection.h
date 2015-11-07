#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <string>
#include <factory.h>
#include <visibility.h>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

namespace DB {
	class SelectCommand;
	class ModifyCommand;
	enum BulkDeleteStyle {
		BulkDeleteUsingSubSelect,
		BulkDeleteUsingUsing,
		BulkDeleteUsingUsingAlias,
	};
	enum BulkUpdateStyle {
		BulkUpdateByIteration,
		BulkUpdateUsingFromSrc,
		BulkUpdateUsingJoin,
	};
	/// Base class for connections to a database.
	class DLL_PUBLIC Connection {
		public:
			virtual ~Connection();

			/// Perform final checks before closing.
			virtual void	finish() const = 0;
			/// Open a new transaction.
			virtual int		beginTx() const = 0;
			/// Commit an open transaction.
			virtual int		commitTx() const = 0;
			/// Rollback an open transaction.
			virtual int		rollbackTx() const = 0;
			/// Test to see if a transaction is currently open.
			virtual bool	inTx() const = 0;
			/// Create a named save point.
			virtual void	savepoint(const std::string &) const;
			/// Rollback to a named save point.
			virtual void	rollbackToSavepoint(const std::string &) const;
			/// Release a named save point.
			virtual void	releaseSavepoint(const std::string &) const;
			/// Test server connection availability.
			virtual void	ping() const = 0;
			/// @cond
			virtual BulkDeleteStyle bulkDeleteStyle() const = 0;
			virtual BulkUpdateStyle bulkUpdateStyle() const = 0;
			/// @endcond

			/// Straight up execute a statement (no access to result set)
			virtual void execute(const std::string & sql) const;
			/// Execute a script from a stream.
			/// @param f the script.
			/// @param s the location of the script.
			virtual void executeScript(std::istream & f, const boost::filesystem::path & s) const;
			/// Create a new select command with the given SQL.
			virtual SelectCommand * newSelectCommand(const std::string & sql) const = 0;
			/// Create a new modify command with the given SQL.
			virtual ModifyCommand * newModifyCommand(const std::string & sql) const = 0;

			/// Begin a bulk upload operation.
			/// @param table the target table.
			/// @param opts database specific options to the load command.
			virtual void beginBulkUpload(const char * table, const char * opts) const = 0;
			/// Finish a bulk upload operation.
			virtual void endBulkUpload(const char *) const = 0;
			/// Load data for the current bulk load operation.
			virtual size_t bulkUploadData(const char *, size_t) const = 0;

			/// Return the Id used in the last insert
			virtual int64_t insertId() const;

			/// AdHoc plugin resolver helper for database connectors.
			static boost::optional<std::string> resolvePlugin(const std::type_info &, const std::string &);
		private:
	};

	typedef boost::shared_ptr<Connection> ConnectionPtr;
	typedef AdHoc::Factory<Connection, std::string> ConnectionFactory;
}

#endif

