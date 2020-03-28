#ifndef TABLEPATCH_H
#define TABLEPATCH_H

#include <string>
#include <set>
#include <map>
#include <connection.h>
#include <modifycommand.h>
#include <selectcommand.h>
#include <functional>

namespace DB {
	class SqlWriter;

	/// Table patch settings.
	class DLL_PUBLIC TablePatch {
		private:
			using TableName = std::string;
			using ColumnName = std::string;
			using ColumnNames = std::set<ColumnName>;
			using PrimaryKey = ColumnNames;
			using PKI = PrimaryKey::const_iterator;
			using AuditFunction = std::function<void(DB::SelectCommandPtr)>;

		public:
			/// Source table name.
			TableName src;
			/// Source expression.
			SqlWriter * srcExpr { nullptr };
			/// Destination table name.
			TableName dest;
			/// Columns comprising the [effective] primary key.
			PrimaryKey pk;
			/// Columns to be updated in the path.
			ColumnNames cols;
			/// An optional SQL writer to replace the default delete operation.
			SqlWriter * insteadOfDelete { nullptr };
			/// An optional SQL writer to append a where clause.
			SqlWriter * where { nullptr };
			/// An optional SQL writer to append an order by clause.
			SqlWriter * order { nullptr };
			/// Enable deletion
			bool doDeletes { true };
			/// Enable updates
			bool doUpdates { true };
			/// Enable insertion
			bool doInserts { true };
			/// Before delete audit
			AuditFunction beforeDelete;
			/// Before update audit
			AuditFunction beforeUpdate;
			/// Before insert audit
			AuditFunction beforeInsert;
	};
}

#endif

