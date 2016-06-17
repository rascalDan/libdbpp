#ifndef DB_SELECTCOMMANDUTIL_IMPL_H
#define DB_SELECTCOMMANDUTIL_IMPL_H

#include "selectcommand.h"
#include <boost/function.hpp>
#include <boost/utility/enable_if.hpp>

/// @cond
namespace DB {
	template<typename Fields, typename Func, unsigned int field, typename ... Fn>
	inline typename boost::disable_if_c<field < std::tuple_size<Fields>::value>::type
	forEachField(DB::SelectCommand *, const Func & func, const Fn & ... args)
	{
		func(args...);
	}

	template<typename Fields, typename Func, unsigned int field, typename ... Fn, typename ... Args>
	inline typename boost::enable_if_c<field < std::tuple_size<Fields>::value>::type
	forEachField(DB::SelectCommand * sel, const Func & func, const Args & ... args)
	{
		typename std::tuple_element<field, Fields>::type a;
		(*sel)[field] >> a;
		forEachField<Fields, Func, field + 1, Fn...>(sel, func, args..., a);
	}

	template<typename ... Fn, typename Func>
	inline void SelectCommand::forEachRow(const Func & func)
	{
		while (fetch()) {
			forEachField<std::tuple<Fn...>, Func, 0>(this, func);
		}
	}
}
/// @endcond

#endif

