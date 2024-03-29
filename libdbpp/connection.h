#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include "command_fwd.h"
#include "error.h"
#include <c++11Helpers.h>
#include <cstdint>
#include <cstdio>
#include <exception.h>
#include <factory.h> // IWYU pragma: keep
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <typeinfo>
#include <visibility.h>
// IWYU pragma: no_include "factory.impl.h"

namespace DB {
	class TablePatch;

	enum class BulkDeleteStyle {
		UsingSubSelect,
		UsingUsing,
		UsingUsingAlias,
	};

	enum class BulkUpdateStyle {
		UsingFromSrc = 1,
		UsingJoin = 2,
	};

	using TableName = std::string;
	using ColumnName = std::string;
	using ColumnNames = std::set<ColumnName>;
	using PrimaryKey = ColumnNames;
	using PKI = PrimaryKey::const_iterator;

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
		std::string message() const noexcept override;
	};

	/// Exception thrown when attempting to open a transaction when one is already open.
	class DLL_PUBLIC TransactionAlreadyOpen : public AdHoc::StdException {
	private:
		std::string message() const noexcept override;
	};

	/// Exception thrown when attempting to perform a table patch with invalid settings.
	class DLL_PUBLIC PatchCheckFailure : public AdHoc::StdException {
	private:
		std::string message() const noexcept override;
	};

	/// Exception thrown when attempting to perform an action that requires a transaction when one is not open.
	class DLL_PUBLIC TransactionRequired : public AdHoc::StdException {
	private:
		std::string message() const noexcept override;
	};

	/// Base class for connections to a database.
	class DLL_PUBLIC Connection : public std::enable_shared_from_this<Connection> {
	public:
		virtual ~Connection() = default;
		/// Standard special members
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(Connection);

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
		virtual void executeScript(std::istream & f, const std::filesystem::path & s);
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
		static std::optional<std::string> resolvePlugin(const std::type_info &, const std::string_view);

	protected:
		/// Create a new connection.
		Connection() = default;

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
		unsigned int txOpenDepth {0};
	};

	/// Helper class for beginning/committing/rolling back transactions in accordance with scope and exceptions.
	class DLL_PUBLIC TransactionScope {
	public:
		/// Create a new helper and associated transaction on the given connection.
		explicit TransactionScope(Connection &);
		~TransactionScope() noexcept;
		/// Standard special members
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(TransactionScope);

	private:
		Connection * conn;
	};

	using ConnectionFactory = AdHoc::Factory<Connection, std::string>;
	using ConnectionFactoryCPtr = std::shared_ptr<const ConnectionFactory>;
}

#endif
