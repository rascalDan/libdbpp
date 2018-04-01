#include "column.h"
#include <exception>
#include <compileTimeFormatter.h>

namespace DB {
Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n.collate_key())
{
}

Column::~Column()
{
}

InvalidConversion::InvalidConversion(const char * const f, const char * const t) : from(f), to(t) { }

AdHocFormatter(InvalidConversionMsg, "Invalid conversion from column type (%?) to value type (%?)");
std::string
InvalidConversion::message() const throw()
{
	return InvalidConversionMsg::get(from, to);
}

void
DB::HandleField::blob(const DB::Blob &)
{
	throw DB::ColumnTypeNotSupported();
}

template<typename T>
class Extract : public DB::HandleField {
	public:
		Extract(T & t) : target(t) { }

		void floatingpoint(double v) override { (*this)(v); }
		void integer(int64_t v) override { (*this)(v); }
		void boolean(bool v) override { (*this)(v); }
		void string(const char * v, size_t len) override { (*this)(std::string(v, len)); }
		void timestamp(const boost::posix_time::ptime & v) override { (*this)(v); }
		void interval(const boost::posix_time::time_duration & v) override { (*this)(v); }
		void blob(const Blob & v) override { (*this)(v); }
		void null() override { }

		template <typename D>
		void operator()(const D & v) {
			if constexpr (std::is_convertible<D, T>::value) {
				target = (T)v;
			}
			else {
				throw InvalidConversion(typeid(D).name(), typeid(T).name());
			}
		}

		T & target;
};

#define COLUMNINTO(Type) \
void \
Column::operator>>(Type & v) const \
{ \
	Extract<Type> e(v); \
	apply(e); \
}

COLUMNINTO(bool);
COLUMNINTO(int64_t);
COLUMNINTO(double);
COLUMNINTO(std::string);
COLUMNINTO(boost::posix_time::ptime);
COLUMNINTO(boost::posix_time::time_duration);
COLUMNINTO(Blob);
}

