#ifndef DB_TESTCORE_H
#define DB_TESTCORE_H

#include <command.h>
#include <visibility.h>

namespace DB {

class SelectCommand;

/// @cond
class DLL_PUBLIC TestCore {
	protected:
		TestCore();

		int64_t testInt { 43 };
		double testDouble { 3.14 };
		std::string_view testString;
		bool testBool { false };
		boost::posix_time::ptime testDateTime;
		boost::posix_time::time_duration testInterval;

		template<typename T> void assertScalarValueHelper(SelectCommand & sel, const T & t) const;
		template<typename T> void assertColumnValueHelper(SelectCommand & sel, unsigned int col, const T & t) const;
};
/// @endcond

DLL_PUBLIC
std::ostream &
operator<<(std::ostream & s, const Blob b);

}

#endif

