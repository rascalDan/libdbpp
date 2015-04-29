#ifndef DB_COLUMN_H
#define DB_COLUMN_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>

namespace DB {
	class HandleField {
		public:
			virtual void null() = 0;
			virtual void string(const char *, size_t len) = 0;
			virtual void integer(int64_t) = 0;
			virtual void floatingpoint(double) = 0;
			virtual void interval(const boost::posix_time::time_duration &) = 0;
			virtual void timestamp(const boost::posix_time::ptime &) = 0;
	};
	class Command;
	class Column {
		public:
			Column(const Glib::ustring &, unsigned int);
			virtual ~Column() = 0;

			virtual bool isNull() const = 0;
			virtual void apply(HandleField &) const = 0;
			virtual void rebind(Command *, unsigned int) const = 0;

			const unsigned int		colNo;
			const Glib::ustring		name;
	};
	typedef boost::shared_ptr<Column> ColumnPtr;
}

#endif

