#ifndef DB_SELECTCOMMANDUTIL_IMPL_H
#define DB_SELECTCOMMANDUTIL_IMPL_H

#include "selectcommand.h"
#include <type_traits>

/// @cond
namespace DB {
	template<typename Fields, typename Func, unsigned int field, typename ... Fn, typename ... Args>
	inline void
	forEachField(DB::SelectCommand * sel, const Func & func, const Args & ... args)
	{
		if constexpr (field >= std::tuple_size<Fields>::value) {
			(void)sel;
			func(args...);
		}
		else {
			typename std::tuple_element<field, Fields>::type a;
			(*sel)[field] >> a;
			forEachField<Fields, Func, field + 1, Fn...>(sel, func, args..., a);
		}
	}

	template<typename ... Fn, typename Func>
	inline void SelectCommand::forEachRow(const Func & func)
	{
		while (fetch()) {
			forEachField<std::tuple<Fn...>, Func, 0>(this, func);
		}
	}

	template<typename ... Fn>
	inline RowRange<Fn...> SelectCommand::as()
	{
		return RowRange<Fn...>(this);
	}

	template<typename ... Fn>
	inline RowRange<Fn...>::RowRange(SelectCommand * s) :
		sel(s)
	{
	}

	template<typename ... Fn>
	inline RowRangeIterator<Fn...> RowRange<Fn...>::begin() const
	{
		return RowRangeIterator<Fn...>(sel);
	}

	template<typename ... Fn>
	inline RowRangeIterator<Fn...> RowRange<Fn...>::end() const
	{
		return RowRangeIterator<Fn...>(nullptr);
	}

	template<typename ... Fn>
	inline RowRangeIterator<Fn...>::RowRangeIterator(SelectCommand * s) :
		sel(s)
	{
		if (sel) {
			validRow = sel->fetch();
		}
		else {
			validRow = false;
		}
	}

	template<typename ... Fn>
	inline bool RowRangeIterator<Fn...>::operator!=(const RowRangeIterator &) const
	{
		return validRow;
	}

	template<typename ... Fn>
	inline void RowRangeIterator<Fn...>::operator++()
	{
		validRow = sel->fetch();
	}

	template<typename ... Fn>
	inline Row<Fn...> RowRangeIterator<Fn...>::operator*() const
	{
		return Row<Fn...>(sel);
	}

	template<typename ... Fn>
	inline Row<Fn...>::Row(SelectCommand * s) :
		RowBase(s)
	{
	}

	template<typename ... Fn>
	template<unsigned int C>
	inline typename std::tuple_element<C, std::tuple<Fn...>>::type Row<Fn...>::value() const
	{
		typename std::tuple_element<C, std::tuple<Fn...>>::type a;
		sel->operator[](C) >> a;
		return a;
	}
}
/// @endcond

#endif

