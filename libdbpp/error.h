#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <stdlib.h>
#include <exception>
#include <visibility.h>

namespace DB {
	/// Base class for database errors.
	class DLL_PUBLIC Error : public virtual std::exception { };
	/// Base class for database connectivity errors.
	class DLL_PUBLIC ConnectionError : public Error {
		public:
			/// Default constructor, sets FailureTime to now.
			ConnectionError();
			/// Construct with a specific failure time.
			ConnectionError(time_t);

			/// The time of connectivity failure.
			const time_t FailureTime;
	};
}

#endif
