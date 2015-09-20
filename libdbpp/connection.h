#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <visibility.h>

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
	class DLL_PUBLIC Connection {
		public:
			virtual ~Connection();

			virtual void	finish() const = 0;
			virtual int		beginTx() const = 0;
			virtual int		commitTx() const = 0;
			virtual int		rollbackTx() const = 0;
			virtual bool	inTx() const = 0;
			virtual void	savepoint(const std::string &) const;
			virtual void	rollbackToSavepoint(const std::string &) const;
			virtual void	releaseSavepoint(const std::string &) const;
			virtual void	ping() const = 0;
			virtual BulkDeleteStyle bulkDeleteStyle() const = 0;
			virtual BulkUpdateStyle bulkUpdateStyle() const = 0;

			virtual void execute(const std::string & sql) const;
			virtual SelectCommand * newSelectCommand(const std::string & sql) const = 0;
			virtual ModifyCommand * newModifyCommand(const std::string & sql) const = 0;

			virtual void beginBulkUpload(const char *, const char *) const = 0;
			virtual void endBulkUpload(const char *) const = 0;
			virtual size_t bulkUploadData(const char *, size_t) const = 0;

		private:
	};
}

#endif

