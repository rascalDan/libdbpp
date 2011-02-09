#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <stdlib.h>
#include <exception>

namespace DB {
	class Error : public virtual std::exception { };
	class ConnectionError : public Error {
		public:
			ConnectionError();
			ConnectionError(time_t);

			const time_t FailureTime;
	};
}

#endif
