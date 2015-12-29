#ifndef DB_COLUMN_H
#define DB_COLUMN_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <visibility.h>
#include <exception.h>
#include "error.h"

namespace DB {
	/// Exception thrown on an attempt to convert betweem incompatible types.
	class DLL_PUBLIC InvalidConversion : public AdHoc::Exception<Error> {
		public:
			InvalidConversion(const char * const, const char * const);

		private:
			std::string message() const throw() override;
			const char * from;
			const char * to;
	};

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
			const std::string		name;
	};
	typedef boost::shared_ptr<Column> ColumnPtr;
}

#endif

