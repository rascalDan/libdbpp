#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <stdlib.h>
#include <exception>
#include <visibility.h>

namespace DB {
	class DLL_PUBLIC Error : public virtual std::exception { };
	class DLL_PUBLIC ConnectionError : public Error {
		public:
			ConnectionError();
			ConnectionError(time_t);

			const time_t FailureTime;
	};
}

#endif
