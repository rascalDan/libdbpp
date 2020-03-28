#ifndef DB_COLUMN_H
#define DB_COLUMN_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <memory>
#include <optional>
#include <visibility.h>
#include <exception.h>
#include "dbTypes.h"
#include "error.h"
#include <c++11Helpers.h>

namespace DB {
	/// Abstract class for something that can handle field data. See Column::apply.
	class DLL_PUBLIC HandleField {
		public:
			/// The field is null.
			virtual void null() = 0;
			/// The field contains text data.
			virtual void string(std::string_view) = 0;
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
			/// The field is a BLOB.
			virtual void blob(const Blob &);
	};

	class Command;
	/// Represents a column in a result set and provides access to the current rows data.
	class DLL_PUBLIC Column {
		public:
			/// Creates a new column with the given name and ordinal.
			Column(const Glib::ustring &, unsigned int);
			virtual ~Column() = 0;
			/// Standard special members
			SPECIAL_MEMBERS_MOVE_RO(Column);

			/// Test if the current value is null.
			[[nodiscard]] virtual bool isNull() const = 0;
			/// Apply a field handler (any sub-class of HandleField)
			virtual void apply(HandleField &) const = 0;

			/// Column handler dealing with trivial (sensible) type conversions
			template<typename T>
			class Extract : public DB::HandleField {
				private:
					template <typename X> struct is_optional {
						static constexpr bool value = false;
						static constexpr bool is_arithmetic = std::is_arithmetic<X>::value;
					};
					template <typename X> struct is_optional<std::optional<X>> {
						static constexpr bool value = true;
						static constexpr bool is_arithmetic = std::is_arithmetic<X>::value;
					};

				public:
					/// Create an extrator given a target variable.
					explicit Extract(T & t) : target(t) { }

					void floatingpoint(double v) override { (*this)(v); }
					void integer(int64_t v) override { (*this)(v); }
					void boolean(bool v) override { (*this)(v); }
					void string(const std::string_view v) override { (*this)(v); }
					void timestamp(const boost::posix_time::ptime & v) override { (*this)(v); }
					void interval(const boost::posix_time::time_duration & v) override { (*this)(v); }
					void blob(const Blob & v) override { (*this)(v); }
					void null() override
					{
						if constexpr (is_optional<T>::value) {
							target.reset();
						}
						else {
							throw UnexpectedNullValue(typeid(T).name());
						}
					}

					/// Default call operation to [convert and] assign field value to target.
					template <typename D>
					inline
					void operator()(const D & v)
					{
						if constexpr (is_optional<T>::is_arithmetic == std::is_arithmetic<D>::value) {
							if constexpr (std::is_assignable<T, D>::value) {
								target = v;
								return;
							}
							if constexpr (std::is_convertible<T, D>::value) {
								target = (T)v;
								return;
							}
						}
						throw InvalidConversion(typeid(D).name(), typeid(T).name());
					}

				private:
					T & target;
			};

			/// STL like extractor.
			template<typename T>
			void operator>>(T & v) const
			{
				Extract<T> e(v);
				apply(e);
			}

			/// This column's ordinal.
			const unsigned int		colNo;
			/// This column's name.
			const std::string		name;
	};
	using ColumnPtr = std::unique_ptr<Column>;
}

#endif

