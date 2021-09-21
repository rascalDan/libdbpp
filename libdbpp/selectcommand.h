#ifndef DB_SELECTCOMMAND_H
#define DB_SELECTCOMMAND_H

#include "column.h"
#include "command.h"
#include "dbTypes.h" // IWYU pragma: keep
#include <array>
#include <c++11Helpers.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop
#include <cstddef>
#include <exception.h>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <visibility.h>

#ifndef BOOST_TEST_MODULE
#	define DEPRECATE __attribute__((deprecated))
#else
#	define DEPRECATE
#endif

namespace DB {
	class Error;
	class SelectCommand;

	/// @cond
	class DLL_PUBLIC RowBase {
	public:
		explicit RowBase(SelectCommand *);

		/// Get a column reference by index.
		const Column & operator[](unsigned int col) const;
		/// Get a column reference by name.
		const Column & operator[](const Glib::ustring &) const;

	protected:
		SelectCommand * sel;
	};

	template<typename... Fn> class Row : public RowBase {
	public:
		explicit Row(SelectCommand *);

		template<unsigned int C> using FieldType = typename std::tuple_element<C, std::tuple<Fn...>>::type;

		/// Get value of column C in current row.
		template<unsigned int C> [[nodiscard]] FieldType<C> value() const DEPRECATE;

		template<unsigned int C> [[nodiscard]] FieldType<C> get() const;
	};

	template<typename... Fn> class RowRangeIterator {
	public:
		explicit RowRangeIterator(SelectCommand *);

		bool operator!=(const RowRangeIterator &) const;
		void operator++();
		Row<Fn...> operator*() const;

	private:
		SelectCommand * sel;
		bool validRow;
	};

	template<typename... Fn> class RowRange {
	public:
		explicit RowRange(SelectCommand *);

		RowRangeIterator<Fn...> begin() const;
		RowRangeIterator<Fn...> end() const;

	private:
		SelectCommand * sel;
	};
	/// @endcond

	/// Exception thrown when the requested column is outside the range of the result set.
	class DLL_PUBLIC ColumnIndexOutOfRange : public AdHoc::Exception<Error> {
	public:
		/// New ColumnIndexOutOfRange exception
		/// @param n Index requested
		explicit ColumnIndexOutOfRange(unsigned int n);

		/// Index requested
		const unsigned int colNo;

	private:
		std::string message() const noexcept override;
	};

	/// Exception thrown when the requested column does not exist in the result set.
	class DLL_PUBLIC ColumnDoesNotExist : public AdHoc::Exception<Error> {
	public:
		/// New ColumnDoesNotExist exception
		/// @param n Name requested
		explicit ColumnDoesNotExist(Glib::ustring n);

		/// Name requested
		const Glib::ustring colName;

	private:
		std::string message() const noexcept override;
	};

	/// Represents a command expected to return data to the client.
	class DLL_PUBLIC SelectCommand : public virtual Command {
	public:
		/// Creates a new command from the given SQL.
		explicit SelectCommand(const std::string & sql);
		~SelectCommand() override;

		/// Standard special members
		SPECIAL_MEMBERS_MOVE_RO(SelectCommand);

		/// Fetch the next row from the result set. Returns false when no further rows are availabile.
		virtual bool fetch() = 0;
		/// Execute the statement, but don't fetch the first row.
		virtual void execute() = 0;
		/// Get a column reference by index.
		[[nodiscard]] const Column & operator[](unsigned int col) const;
		/// Get a column reference by name.
		[[nodiscard]] const Column & operator[](const Glib::ustring &) const;
		/// Get the number of columns in the result set.
		[[nodiscard]] unsigned int columnCount() const;
		/// Get the index of a column by name.
		[[nodiscard]] unsigned int getOrdinal(const Glib::ustring &) const;
		/// Push each row through a function accepting one value per column
		template<typename... Fn, typename Func = std::function<void(Fn...)>> void forEachRow(const Func & func);
		/// Support for a C++ row range for
		template<typename... Fn> RowRange<Fn...> as();

	protected:
		/// Helper function so clients need not know about boost::multi_index_container.
		const ColumnPtr & insertColumn(ColumnPtr);

		class Columns;

		/// Columns in the result set.
		std::unique_ptr<Columns> columns;
	};
}

namespace std {
	/// @cond
	template<typename... Fn> struct tuple_size<DB::Row<Fn...>> {
		static constexpr auto value = sizeof...(Fn);
	};
	template<size_t C, typename... Fn> struct tuple_element<C, DB::Row<Fn...>> {
		using type = typename std::tuple_element<C, std::tuple<Fn...>>::type;
	};
	/// @endcond
}

#endif
