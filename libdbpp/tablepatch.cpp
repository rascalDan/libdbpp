#include "connection.h"
#include "modifycommand.h"
#include "selectcommand.h"
#include "tablepatch.h"
#include "sqlWriter.h"
#include <buffer.h>
#include <safeMapFind.h>
#include <boost/algorithm/string/join.hpp>

DB::TablePatch::TablePatch() :
	insteadOfDelete(nullptr),
	where(nullptr),
	order(nullptr),
	doDeletes(true),
	doUpdates(true),
	doInserts(true)
{
}

DB::PatchResult
DB::Connection::patchTable(TablePatch * tp)
{
	if (tp->pk.empty()) {
		throw PatchCheckFailure();
	}
	if (!inTx()) {
		throw TransactionRequired();
	}
	TransactionScope tx(this);
	return {
		tp->doDeletes ? patchDeletes(tp) : 0,
		tp->doUpdates ? patchUpdates(tp) : 0,
		tp->doInserts ? patchInserts(tp) : 0
	};
}

template<typename Container>
static inline void
push(const boost::format &, typename Container::const_iterator &)
{
}

template<typename Container, typename Value, typename ... Values>
static inline void
push(boost::format & f, typename Container::const_iterator & i, const Value & v, const Values & ... vs)
{
	f % v(i);
	push<Container>(f, i, vs...);
}

template<typename Separator, typename Container, typename ... Ps>
static inline unsigned int
appendIf(AdHoc::Buffer & buf, const Container & c, const boost::function<bool(const typename Container::const_iterator)> & sel, const Separator & sep, const std::string & fmts, const Ps & ... ps)
{
	auto fmt = AdHoc::Buffer::getFormat(fmts);
	unsigned int x = 0;
	for (typename Container::const_iterator i = c.begin(); i != c.end(); ++i) {
		if (sel(i)) {
			if (x > 0) {
				buf.appendbf("%s", sep);
			}
			push<Container>(*fmt, i, ps...);
			buf.append(fmt->str());
			x += 1;
		}
	}
	return x;
}

template<typename Separator, typename Container, typename ... Ps>
static inline unsigned int
append(AdHoc::Buffer & buf, const Container & c, const Separator & sep, const std::string & fmts, const Ps & ... ps)
{
	return appendIf(buf, c, [](auto){ return true; }, sep, fmts, ps...);
}

template <typename Container>
static inline typename Container::key_type
self(const typename Container::const_iterator & i)
{
	return *i;
}
#define selfCols self<decltype(TablePatch::cols)>
#define isKey(tp) [tp](auto i){ return AdHoc::containerContains(tp->pk, *i); }
#define isNotKey(tp) [tp](auto i){ return !AdHoc::containerContains(tp->pk, *i); }

unsigned int
DB::Connection::patchDeletes(TablePatch * tp)
{
	AdHoc::Buffer toDelSql;
	switch (bulkDeleteStyle()) {
		case BulkDeleteUsingSubSelect:
			{
				// -----------------------------------------------------------------
				// Build SQL to delete keys ----------------------------------------
				// -----------------------------------------------------------------
				if (tp->insteadOfDelete) {
					toDelSql.appendbf("UPDATE %s SET ",
							tp->dest);
					tp->insteadOfDelete->writeSql(toDelSql);
					toDelSql.append(" WHERE (");
				}
				else {
					toDelSql.appendbf("DELETE FROM %s WHERE (",
							tp->dest);
				}
				append(toDelSql, tp->pk, ", ", "%s.%s", [tp](auto){ return tp->dest; }, selfCols);
				// -----------------------------------------------------------------
				// Build SQL to select keys to delete ------------------------------
				// -----------------------------------------------------------------
				toDelSql.append(") IN (SELECT ");
				append(toDelSql, tp->pk, ", ", "a.%s", selfCols);
				toDelSql.appendbf(" FROM %s a LEFT OUTER JOIN %s b ON ",
						tp->dest, tp->src);
				append(toDelSql, tp->pk, " AND ", " a.%s = b.%s", selfCols, selfCols);
				toDelSql.append(" WHERE ");
				append(toDelSql, tp->pk, " AND ", " b.%s IS NULL", [](auto & pki){ return *pki; });
				if (tp->where) {
					toDelSql.append(" AND ");
					tp->where->writeSql(toDelSql);
				}
				if (tp->order) {
					toDelSql.append(" ORDER BY ");
					tp->order->writeSql(toDelSql);
				}
				toDelSql.append(")");
				break;
			}
		case BulkDeleteUsingUsingAlias:
		case BulkDeleteUsingUsing:
			{
				if (tp->insteadOfDelete) {
					toDelSql.appendbf("UPDATE %s a ",
							tp->dest);
				}
				else {
					toDelSql.appendbf("DELETE FROM %s USING %s a ",
							(bulkDeleteStyle() == BulkDeleteUsingUsingAlias ? "a" : tp->dest),
							tp->dest);
				}
				toDelSql.appendbf(" LEFT OUTER JOIN %s b ",
						tp->src);
				toDelSql.append(" ON ");
				append(toDelSql, tp->pk, " AND ", " a.%s = b.%s ", selfCols, selfCols);
				if (tp->insteadOfDelete) {
					tp->insteadOfDelete->writeSql(toDelSql);
				}
				toDelSql.append(" WHERE ");
				append(toDelSql, tp->pk, " AND ", " b.% IS NULL", selfCols);
				if (tp->where) {
					toDelSql.append(" AND ");
					tp->where->writeSql(toDelSql);
				}
				if (tp->order) {
					toDelSql.append(" ORDER BY ");
					tp->order->writeSql(toDelSql);
				}
				break;
			}
	}
	auto del = ModifyCommandPtr(newModifyCommand(toDelSql));
	unsigned int offset = 0;
	if (tp->insteadOfDelete) {
		tp->insteadOfDelete->bindParams(del.get(), offset);
	}
	if (tp->where) {
		tp->where->bindParams(del.get(), offset);
	}
	if (tp->order) {
		tp->order->bindParams(del.get(), offset);
	}
	return del->execute();
}

unsigned int
DB::Connection::patchUpdates(TablePatch * tp)
{
	if (tp->cols.size() == tp->pk.size()) {
		// Can't "change" anything... it's all part of the key
		return 0;
	}
	switch (bulkUpdateStyle()) {
		case BulkUpdateUsingFromSrc:
			{
				// -----------------------------------------------------------------
				// Build SQL for list of updates to perform ------------------------
				// -----------------------------------------------------------------
				AdHoc::Buffer updSql;
				updSql.appendbf("UPDATE %s a SET ",
						tp->dest);
				appendIf(updSql, tp->cols, isNotKey(tp), ", ", " %s = b.%s ", selfCols, selfCols);
				updSql.appendbf(" FROM %s b ",
						tp->src);
				updSql.append(" WHERE ");
				append(updSql, tp->pk, " AND " , " a.%s = b.%s ", selfCols, selfCols);
				updSql.append(" AND (");
				appendIf(updSql, tp->cols, isNotKey(tp), "  OR ",
								" (((CASE WHEN (a.%s IS NULL AND b.%s IS NULL) THEN 1 ELSE 0 END) \
							+ (CASE WHEN(a.%s = b.%s) THEN 1 ELSE 0 END)) = 0)",
							selfCols, selfCols, selfCols, selfCols);
				updSql.append(")");
				if (tp->where) {
					updSql.append(" AND ");
					tp->where->writeSql(updSql);
				}
				// -----------------------------------------------------------------
				// Execute the bulk update command ---------------------------------
				// -----------------------------------------------------------------
				auto upd = ModifyCommandPtr(newModifyCommand(updSql));
				unsigned int offset = 0;
				if (tp->where) {
					tp->where->bindParams(upd.get(), offset);
				}
				return upd->execute(true);
			}
			break;
		case BulkUpdateUsingJoin:
			{
				// -----------------------------------------------------------------
				// Build SQL for list of updates to perform ------------------------
				// -----------------------------------------------------------------
				AdHoc::Buffer updSql;
				updSql.appendbf("UPDATE %s a, %s b SET ",
						tp->dest, tp->src);
				appendIf(updSql, tp->cols, isNotKey(tp), ", ", " a.%s = b.%s ", selfCols, selfCols);
				updSql.append(" WHERE ");
				append(updSql, tp->pk, " AND ", " a.%s = b.%s ", selfCols, selfCols);
				updSql.append(" AND (");
				appendIf(updSql, tp->cols, isNotKey(tp), " OR ",
						" (((CASE WHEN (a.%s IS NULL AND b.%s IS NULL) THEN 1 ELSE 0 END) \
						+ (CASE WHEN(a.%s = b.%s) THEN 1 ELSE 0 END)) = 0)",
						selfCols, selfCols, selfCols, selfCols);
				updSql.append(")");
				if (tp->where) {
					updSql.append(" AND ");
					tp->where->writeSql(updSql);
				}
				if (tp->order) {
					updSql.append(" ORDER BY ");
					tp->order->writeSql(updSql);
				}
				// -----------------------------------------------------------------
				// Execute the bulk update command ---------------------------------
				// -----------------------------------------------------------------
				auto upd = ModifyCommandPtr(newModifyCommand(updSql));
				unsigned int offset = 0;
				if (tp->where) {
					tp->where->bindParams(upd.get(), offset);
				}
				if (tp->order) {
					tp->order->bindParams(upd.get(), offset);
				}
				return upd->execute(true);
			}
		default:
			return 0;
	}
}

unsigned int
DB::Connection::patchInserts(TablePatch * tp)
{
	// -----------------------------------------------------------------
	// Build SQL for copying new records -------------------------------
	// -----------------------------------------------------------------
	AdHoc::Buffer toInsSql;
	toInsSql.appendbf("INSERT INTO %s(",
			tp->dest);
	append(toInsSql, tp->cols, ", ", "%s", selfCols);
	toInsSql.append(") SELECT ");
	append(toInsSql, tp->cols, ", ", "b.%s", selfCols);
	toInsSql.appendbf(" FROM %s b LEFT OUTER JOIN %s a ON ",
			tp->src, tp->dest);
	append(toInsSql, tp->pk, " AND ", " a.%s = b.%s", selfCols, selfCols);
	toInsSql.append(" WHERE ");
	append(toInsSql, tp->pk, " AND ", " a.%s IS NULL", selfCols);
	if (tp->order) {
		toInsSql.appendf(" ORDER BY ");
		tp->order->writeSql(toInsSql);
	}
	auto ins = ModifyCommandPtr(newModifyCommand(toInsSql));
	if (tp->order) {
		unsigned int offset = 0;
		tp->order->bindParams(ins.get(), offset);
	}
	return ins->execute();
}

std::string
DB::PatchCheckFailure::message() const throw()
{
	return "Santiy checks failed: check table names and keys";
}

