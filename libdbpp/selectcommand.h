#ifndef DB_SELECTCOMMAND_H
#define DB_SELECTCOMMAND_H

#include "command.h"
#include "column.h"
#include "error.h"
#include <functional>
#include <visibility.h>
#include <exception.h>

#ifndef BOOST_TEST_MODULE
#define DEPRECATE __attribute__((deprecated))
#else
#define DEPRECATE
#endif

namespace DB {
	class Column;
	class SelectCommand;

	/// @cond
	class DLL_PUBLIC RowBase {
		public:
			RowBase(SelectCommand *);

			/// Get a column reference by index.
			const Column & operator[](unsigned int col) const;
			/// Get a column reference by name.
			const Column & operator[](const Glib::ustring &) const;

		protected:
			SelectCommand * sel;
	};

	template<typename ... Fn>
	class Row : public RowBase {
		public:
			Row(SelectCommand *);

			/// Get value of column C in current row.
			template<unsigned int C>
			typename std::tuple_element<C, std::tuple<Fn...>>::type value() const DEPRECATE ;

			template<unsigned int C>
			typename std::tuple_element<C, std::tuple<Fn...>>::type get() const;
	};

	template<typename ... Fn>
	class RowRangeIterator {
		public:
			RowRangeIterator(SelectCommand *);

			bool operator!=(const RowRangeIterator &) const;
			void operator++();
			Row<Fn...> operator*() const;

		private:
			SelectCommand * sel;
			bool validRow;
	};

	template<typename ... Fn>
	class RowRange {
		public:
			RowRange(SelectCommand *);

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
			ColumnIndexOutOfRange(unsigned int n);

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
			ColumnDoesNotExist(Glib::ustring n);

			/// Name requested
			const Glib::ustring colName;

		private:
			std::string message() const noexcept override;
	};

	/// Represents a command expected to return data to the client.
	class DLL_PUBLIC SelectCommand : public virtual Command {
		public:
			/// Creates a new command from the given SQL.
			SelectCommand(const std::string & sql);
			~SelectCommand();

			/// Fetch the next row from the result set. Returns false when no further rows are availabile.
			virtual bool fetch() = 0;
			/// Execute the statement, but don't fetch the first row.
			virtual void execute() = 0;
			/// Get a column reference by index.
			const Column & operator[](unsigned int col) const;
			/// Get a column reference by name.
			const Column & operator[](const Glib::ustring &) const;
			/// Get the number of columns in the result set.
			unsigned int columnCount() const;
			/// Get the index of a column by name.
			unsigned int getOrdinal(const Glib::ustring &) const;
			/// Push each row through a function accepting one value per column
			template<typename ... Fn, typename Func = std::function<void(Fn...)>>
			void forEachRow(const Func & func);
			/// Support for a C++ row range for
			template<typename ... Fn>
			RowRange<Fn...> as();

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
	template<typename ... Fn> struct tuple_size<DB::Row<Fn...>> {
		static constexpr auto value =  sizeof...(Fn);
	};
	template<size_t C, typename ... Fn> struct tuple_element<C, DB::Row<Fn...>> {
		typedef typename std::tuple_element<C, std::tuple<Fn...>>::type type;
	};
	/// @endcond
}

#endif

