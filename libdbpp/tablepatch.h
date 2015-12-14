#ifndef TABLEPATCH_H
#define TABLEPATCH_H

#include <string>
#include <set>
#include <map>
#include <connection.h>
#include <modifycommand.h>
#include <selectcommand.h>

class TablePatch {
	public:


	private:
		void            doDeletes(DynamicSql::SqlWriterPtr insteadOfDelete, DynamicSql::SqlWriterPtr where, DynamicSql::SqlWriterPtr order);
		void            doUpdates(DynamicSql::SqlWriterPtr where, DynamicSql::SqlWriterPtr order);
		void            doInserts(DynamicSql::SqlWriterPtr order);
};

#endif

