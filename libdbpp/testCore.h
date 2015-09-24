#ifndef DB_TESTCORE_H
#define DB_TESTCORE_H

#include <command.h>
#include <visibility.h>

namespace DB {

class SelectCommand;

class DLL_PUBLIC TestCore {
	protected:
		TestCore();

		int64_t testInt;
		double testDouble;
		std::string testString;
		bool testBool;
		boost::posix_time::ptime testDateTime;
		boost::posix_time::time_duration testInterval;

		template<typename T> void assertScalarValueHelper(SelectCommand & sel, const T & t) const;
		template<typename T> void assertColumnValueHelper(SelectCommand & sel, unsigned int col, const T & t) const;
};

}

#endif
