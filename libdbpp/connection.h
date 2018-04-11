#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include "connection_fwd.h"
#include <string>
#include <set>
#include <factory.h>
#include <exception.h>
#include <visibility.h>
#include <boost/filesystem/path.hpp>
#include <memory>
#include <optional>
#include "error.h"

namespace AdHoc {
	class Buffer;
}

namespace DB {
	class Command;
	class CommandOptions;
	typedef std::shared_ptr<CommandOptions> CommandOptionsPtr;
	typedef std::shared_ptr<const CommandOptions> CommandOptionsCPtr;
	class SelectCommand;
	typedef std::shared_ptr<SelectCommand> SelectCommandPtr;
	class ModifyCommand;
	typedef std::shared_ptr<ModifyCommand> ModifyCommandPtr;
	class TablePatch;

	enum BulkDeleteStyle {
		BulkDeleteUsingSubSelect,
		BulkDeleteUsingUsing,
		BulkDeleteUsingUsingAlias,
	};

	enum BulkUpdateStyle {
		BulkUpdateUsingFromSrc = 1,
		BulkUpdateUsingJoin = 2,
	};

	typedef std::string TableName;
	typedef std::string ColumnName;
	typedef std::set<ColumnName> ColumnNames;
	typedef ColumnNames PrimaryKey;
	typedef PrimaryKey::const_iterator PKI;

	/// Result of a table patch operation.
	struct PatchResult {
		/// Number of rows deleted.
		unsigned int deletes;
		/// Number of rows updated.
		unsigned int updates;
		/// Number of rows inserted.
		unsigned int inserts;
	};

	/// Base class for database connectivity errors.
	class DLL_PUBLIC ConnectionError : public Error {
		public:
			/// Default constructor, sets FailureTime to now.
			ConnectionError();

			/// The time of connectivity failure.
			const time_t FailureTime;
	};

	/// Exception thrown when finishing a connection that still has a transaction open.
	class DLL_PUBLIC TransactionStillOpen : public AdHoc::StdException {
		private:
			std::string message() const throw() override;
	};

	/// Exception thrown when attempting to open a transaction when one is already open.
	class DLL_PUBLIC TransactionAlreadyOpen : public AdHoc::StdException {
		private:
			std::string message() const throw() override;
	};

	/// Exception thrown when attempting to perform a table patch with invalid settings.
	class DLL_PUBLIC PatchCheckFailure : public AdHoc::StdException {
		private:
			std::string message() const throw() override;
	};

	/// Exception thrown when attempting to perform an action that requires a transaction when one is not open.
	class DLL_PUBLIC TransactionRequired : public AdHoc::StdException {
		private:
			std::string message() const throw() override;
	};

	/// Base class for connections to a database.
	class DLL_PUBLIC Connection : public std::enable_shared_from_this<Connection> {
		public:
			virtual ~Connection();

			/// Perform final checks before closing.
			void finish() const;
			/// Open a new transaction.
			void beginTx();
			/// Commit an open transaction.
			void commitTx();
			/// Rollback an open transaction.
			void rollbackTx();
			/// Test to see if a transaction is currently open.
			bool inTx() const;
			/// Create a named save point.
			virtual void savepoint(const std::string &);
			/// Rollback to a named save point.
			virtual void rollbackToSavepoint(const std::string &);
			/// Release a named save point.
			virtual void releaseSavepoint(const std::string &);
			/// Test server connection availability.
			virtual void ping() const = 0;
			/// @cond
			virtual BulkDeleteStyle bulkDeleteStyle() const = 0;
			virtual BulkUpdateStyle bulkUpdateStyle() const = 0;
			/// @endcond

			/// Straight up execute a statement (no access to result set)
			virtual void execute(const std::string & sql, const CommandOptionsCPtr & = nullptr);
			/// Execute a script from a stream.
			/// @param f the script.
			/// @param s the location of the script.
			virtual void executeScript(std::istream & f, const boost::filesystem::path & s);
			/// Create a new select command with the given SQL.
			virtual SelectCommandPtr select(const std::string & sql, const CommandOptionsCPtr & = nullptr) = 0;
			/// Create a new modify command with the given SQL.
			virtual ModifyCommandPtr modify(const std::string & sql, const CommandOptionsCPtr & = nullptr) = 0;

			/// Begin a bulk upload operation.
			/// @param table the target table.
			/// @param opts database specific options to the load command.
			virtual void beginBulkUpload(const char * table, const char * opts);
			/// Finish a bulk upload operation.
			virtual void endBulkUpload(const char *);
			/// Load data for the current bulk load operation.
			virtual size_t bulkUploadData(const char *, size_t) const;
			/// Load bulk data from a file (wrapper)
			size_t bulkUploadData(std::istream &) const;
			/// Load bulk data from a file (wrapper)
			size_t bulkUploadData(FILE *) const;

			/// Return the Id used in the last insert
			virtual int64_t insertId();

			/// Patch one table's contents into another.
			PatchResult patchTable(TablePatch * tp);

			/// AdHoc plugin resolver helper for database connectors.
			static std::optional<std::string> resolvePlugin(const std::type_info &, const std::string &);

		protected:
			/// Create a new connection.
			Connection();

			/// Internal begin transaction.
			virtual void beginTxInt() = 0;
			/// Internal commit transaction.
			virtual void commitTxInt() = 0;
			/// Internal rollbacj transaction.
			virtual void rollbackTxInt() = 0;

			/// Internal perform table patch delete operations.
			virtual unsigned int patchDeletes(TablePatch * tp);
			/// Internal perform table patch update operations.
			virtual unsigned int patchUpdates(TablePatch * tp);
			/// Internal perform table patch insert operations.
			virtual unsigned int patchInserts(TablePatch * tp);

		private:
			unsigned int txOpenDepth;
	};

	/// Helper class for beginning/committing/rolling back transactions in accordance with scope and exceptions.
	class DLL_PUBLIC TransactionScope {
		public:
			/// Create a new helper and associated transaction on the given connection.
			TransactionScope(Connection &);
			~TransactionScope();

		private:
			TransactionScope(const TransactionScope &) = delete;
			void operator=(const TransactionScope &) = delete;

			Connection & conn;
	};

	typedef AdHoc::Factory<Connection, std::string> ConnectionFactory;
	typedef std::shared_ptr<const ConnectionFactory> ConnectionFactoryCPtr;
}

#endif

