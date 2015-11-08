#ifndef DB_SELECTCOMMAND_H
#define DB_SELECTCOMMAND_H

#include "command.h"
#include "column.h"
#include "error.h"
#include <boost/multi_index_container_fwd.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index_fwd.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/function/function_fwd.hpp>
#include <boost/shared_ptr.hpp>
#include <visibility.h>

namespace DB {
	class Column;
	class ColumnIndexOutOfRange : public Error {
		public:
			ColumnIndexOutOfRange(unsigned int n);

			const unsigned int colNo;
	};
	class ColumnDoesNotExist : public Error {
		public:
			ColumnDoesNotExist(const Glib::ustring & n);

			const Glib::ustring colName;
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
			template<typename ... Fn, typename Func = boost::function<void(Fn...)>>
			void forEachRow(const Func & func);

		protected:
			/// Helper function so clients need not know about boost::multi_index_container.
			ColumnPtr insertColumn(ColumnPtr);

			/// Friendly typedef cos boost::multi_index_container definitions are massive.
			typedef boost::multi_index_container<ColumnPtr, boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const unsigned int, &DB::Column::colNo>>,
				boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const Glib::ustring, &DB::Column::name>>
								>> Columns;

			/// Columns in the result set.
			Columns * columns;
	};
	typedef boost::shared_ptr<SelectCommand> SelectCommandPtr;
}

#endif

