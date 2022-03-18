#ifndef DB_SELECTCOMMANDUTIL_IMPL_H
#define DB_SELECTCOMMANDUTIL_IMPL_H

#include "selectcommand.h"
#include <type_traits>

/// @cond
namespace DB {
	template<typename... Fn, std::size_t... I>
	inline void
	forEachField(DB::SelectCommand * sel [[maybe_unused]], auto && func, std::index_sequence<I...>)
	{
		std::tuple<Fn...> values;
		(((*sel)[I] >> std::get<I>(values)), ...);
		std::apply(func, values);
	}

	template<typename... Fn, typename Func>
	inline void
	SelectCommand::forEachRow(const Func & func)
	{
		while (fetch()) {
			forEachField<Fn...>(this, func, std::make_index_sequence<sizeof...(Fn)> {});
		}
	}

	template<typename... Fn>
	inline RowRange<Fn...>
	SelectCommand::as()
	{
		return RowRange<Fn...>(this);
	}

	template<typename... Fn> inline RowRange<Fn...>::RowRange(SelectCommand * s) : sel(s) { }

	template<typename... Fn>
	inline RowRangeIterator<Fn...>
	RowRange<Fn...>::begin() const
	{
		return RowRangeIterator<Fn...>(sel);
	}

	template<typename... Fn>
	inline RowRangeIterator<Fn...>
	RowRange<Fn...>::end() const
	{
		return RowRangeIterator<Fn...>(nullptr);
	}

	template<typename... Fn> inline RowRangeIterator<Fn...>::RowRangeIterator(SelectCommand * s) : sel(s)
	{
		if (sel) {
			validRow = sel->fetch();
		}
		else {
			validRow = false;
		}
	}

	template<typename... Fn>
	inline bool
	RowRangeIterator<Fn...>::operator!=(const RowRangeIterator &) const
	{
		return validRow;
	}

	template<typename... Fn>
	inline void
	RowRangeIterator<Fn...>::operator++()
	{
		validRow = sel->fetch();
	}

	template<typename... Fn>
	inline Row<Fn...>
	RowRangeIterator<Fn...>::operator*() const
	{
		return Row<Fn...>(sel);
	}

	template<typename... Fn> inline Row<Fn...>::Row(SelectCommand * s) : RowBase(s) { }

	template<typename... Fn>
	template<unsigned int C>
	inline typename Row<Fn...>::template FieldType<C>
	Row<Fn...>::value() const
	{
		return get<C>();
	}

	template<typename... Fn>
	template<unsigned int C>
	inline typename Row<Fn...>::template FieldType<C>
	Row<Fn...>::get() const
	{
		FieldType<C> a;
		sel->operator[](C) >> a;
		return a;
	}
}
/// @endcond

#endif
