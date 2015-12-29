#ifndef DB_SQLWRITER_H
#define DB_SQLWRITER_H

#include <visibility.h>
#include <buffer.h>

namespace DB {
	class Command;

	/// Base class of dynamic SQL constructors.
	class DLL_PUBLIC SqlWriter {
		public:
			/// Append your SQL to the buffer.
			/// @param buffer The buffer
			virtual void writeSql(AdHoc::Buffer & buffer) = 0;
			/// Bind your parameters (offset should be increment 1 per bind made)
			/// @param cmd Command to bind to.
			/// @param offset The current bind offset.
			virtual void bindParams(Command * cmd, unsigned int & offset);
	};
}

#endif

