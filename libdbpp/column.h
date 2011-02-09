#ifndef DB_COLUMN_H
#define DB_COLUMN_H

#include <glibmm/ustring.h>
#include <vector>
#include <stdlib.h>

namespace DB {
	class HandleField {
		public:
			virtual void null() = 0;
			virtual void string(const char *, size_t len) = 0;
			virtual void integer(int64_t) = 0;
			virtual void floatingpoint(double) = 0;
			virtual void timestamp(const struct tm &) = 0;
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
}

#endif

