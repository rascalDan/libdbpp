#include "column.h"
#include <boost/utility/enable_if.hpp>
#include <exception>
#include <buffer.h>

namespace DB {
Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n)
{
}

Column::~Column()
{
}

InvalidConversion::InvalidConversion(const char * const f, const char * const t) : from(f), to(t) { }

std::string
InvalidConversion::message() const throw()
{
	return stringf("Invalid conversion from column type (%s) to value type (%s)", from, to);
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
		void null() override { }

		template <typename D, typename dummy = int>
		void operator()(const D &,
				typename boost::disable_if<std::is_convertible<D, T>, dummy>::type = 0) {
			throw InvalidConversion(typeid(D).name(), typeid(T).name());
		}

		template <typename D, typename dummy = int>
		void operator()(const D & v,
				typename boost::enable_if<std::is_convertible<D, T>, dummy>::type = 0) {
			target = (T)v;
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
}

