#ifndef DB_SELECTCOMMAND_H
#define DB_SELECTCOMMAND_H

#include "command.h"

namespace DB {
	class Column;
	class SelectCommand : public virtual Command {
		public:
			SelectCommand(const std::string & sql);
			~SelectCommand();
			virtual bool			fetch() = 0;
			virtual void			execute() = 0;
			virtual const Column &	operator[](unsigned int col) const = 0;
			virtual const Column &	operator[](const Glib::ustring &) const = 0;
			virtual unsigned int	columnCount() const = 0;
			virtual unsigned int	getOrdinal(const Glib::ustring &) const = 0;
	};
}

#endif

