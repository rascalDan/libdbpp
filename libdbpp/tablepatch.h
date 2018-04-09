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
			typedef std::string TableName;
			typedef std::string ColumnName;
			typedef std::set<ColumnName> ColumnNames;
			typedef ColumnNames PrimaryKey;
			typedef PrimaryKey::const_iterator PKI;
			typedef std::function<void(DB::SelectCommandPtr)> AuditFunction;

		public:
			/// Default constructor
			TablePatch();

			/// Source table name.
			TableName src;
			/// Source expression.
			SqlWriter * srcExpr;
			/// Destination table name.
			TableName dest;
			/// Columns comprising the [effective] primary key.
			PrimaryKey pk;
			/// Columns to be updated in the path.
			ColumnNames cols;
			/// An optional SQL writer to replace the default delete operation.
			SqlWriter * insteadOfDelete;
			/// An optional SQL writer to append a where clause.
			SqlWriter * where;
			/// An optional SQL writer to append an order by clause.
			SqlWriter * order;
			/// Enable deletion
			bool doDeletes;
			/// Enable updates
			bool doUpdates;
			/// Enable insertion
			bool doInserts;
			/// Before delete audit
			AuditFunction beforeDelete;
			/// Before update audit
			AuditFunction beforeUpdate;
			/// Before insert audit
			AuditFunction beforeInsert;
	};
}

#endif

