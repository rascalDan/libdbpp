#ifndef DB_COLUMN_H
#define DB_COLUMN_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <visibility.h>

namespace DB {
	/// Abstract class for something that can handle field data. See Column::apply.
	class DLL_PUBLIC HandleField {
		public:
			/// The field is null.
			virtual void null() = 0;
			/// The field contains text data.
			virtual void string(const char *, size_t len) = 0;
			/// The field is an integer.
			virtual void integer(int64_t) = 0;
			/// The field is a boolean/bit.
			virtual void boolean(bool) = 0;
			/// The field is floating point/fixed point/numeric.
			virtual void floatingpoint(double) = 0;
			/// The field is an interval/duration/time span.
			virtual void interval(const boost::posix_time::time_duration &) = 0;
			/// The field is a timestamp/date/datetime.
			virtual void timestamp(const boost::posix_time::ptime &) = 0;
	};

	class Command;
	/// Represents a column in a result set and provides access to the current rows data.
	class DLL_PUBLIC Column {
		public:
			/// Creates a new column with the given name and ordinal.
			Column(const Glib::ustring &, unsigned int);
			virtual ~Column() = 0;

			/// Test if the current value is null.
			virtual bool isNull() const = 0;
			/// Apply a field handler (any sub-class of HandleField)
			virtual void apply(HandleField &) const = 0;
			/// Rebind this field to another command (limited support).
			virtual void rebind(Command *, unsigned int) const = 0;

			/// STL like string extractor.
			void operator>>(std::string &) const;
			/// STL like boolean extractor.
			void operator>>(bool &) const;
			/// STL like integer extractor.
			void operator>>(int64_t &) const;
			/// STL like numeric extractor.
			void operator>>(double &) const;
			/// STL like duration extractor.
			void operator>>(boost::posix_time::time_duration &) const;
			/// STL like date time extractor.
			void operator>>(boost::posix_time::ptime &) const;
			template <typename T>
			void operator>>(boost::optional<T> & v) const {
				if (!isNull()) {
					v = T();
					operator>>(v.get());
				}
				else {
					v = boost::none;
				}
			}

			/// This column's ordinal.
			const unsigned int		colNo;
			/// This column's name.
			const Glib::ustring		name;
	};
	typedef boost::shared_ptr<Column> ColumnPtr;
}

#endif

