#ifndef DB_SELECTCOMMAND_H
#define DB_SELECTCOMMAND_H

#include "command.h"
#include "column.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace DB {
	class Column;
	class SelectCommand : public virtual Command {
		public:
			SelectCommand(const std::string & sql);
			~SelectCommand();
			virtual bool fetch() = 0;
			virtual void execute() = 0;
			const Column & operator[](unsigned int col) const;
			const Column & operator[](const Glib::ustring &) const;
			unsigned int columnCount() const;
			unsigned int getOrdinal(const Glib::ustring &) const;

			typedef boost::multi_index_container<ColumnPtr, boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const unsigned int, &DB::Column::colNo>>,
				boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const Glib::ustring, &DB::Column::name>>
								>> Columns;

		protected:
			Columns columns;
	};
}

#endif

