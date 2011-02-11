#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

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
	class Connection {
		public:
			virtual ~Connection();

			virtual int		beginTx() const = 0;
			virtual int		commitTx() const = 0;
			virtual int		rollbackTx() const = 0;
			virtual bool	inTx() const = 0;
			virtual void	ping() const = 0;
			virtual BulkDeleteStyle bulkDeleteStyle() const = 0;
			virtual BulkUpdateStyle bulkUpdateStyle() const = 0;

			virtual SelectCommand * newSelectCommand(const std::string & sql) const = 0;
			virtual ModifyCommand * newModifyCommand(const std::string & sql) const = 0;

		private:
	};
}

#endif

