#ifndef DB_SQLWRITER_H
#define DB_SQLWRITER_H

#include <c++11Helpers.h>
#include <string>
#include <visibility.h>

namespace AdHoc {
	class Buffer;
}
namespace DB {
	class Command;

	/// Base class of dynamic SQL constructors.
	class DLL_PUBLIC SqlWriter {
	public:
		SqlWriter() = default;
		virtual ~SqlWriter() = default;
		/// Standd special memeber
		SPECIAL_MEMBERS_DEFAULT(SqlWriter);
		/// Append your SQL to the buffer.
		/// @param buffer The buffer
		virtual void writeSql(AdHoc::Buffer & buffer) = 0;
		/// Bind your parameters (offset should be increment 1 per bind made)
		/// @param cmd Command to bind to.
		/// @param offset The current bind offset.
		virtual void bindParams(Command * cmd, unsigned int & offset);
	};

	/// A SQL Writer implementation that just writes static SQL.
	class DLL_PUBLIC StaticSqlWriter : public SqlWriter {
	public:
		/// Construct with the SQL to write.
		/// @param sql The SQL to write.
		explicit StaticSqlWriter(std::string sql);
		/// Append the SQL to the buffer.
		/// @param buffer The buffer
		void writeSql(AdHoc::Buffer & buffer) override;

		/// The SQL to write.
		std::string sql;
	};
}

#endif
