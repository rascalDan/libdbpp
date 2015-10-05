#ifndef DB_COMMAND_H
#define DB_COMMAND_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <visibility.h>

namespace DB {
	/// Represents the basics of any command to be executed against a database.
	class DLL_PUBLIC Command {
		public:
			/// Creates a new command from the given SQL.
			Command(const std::string & sql);
			virtual ~Command() = 0;

			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, long val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, long long val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned long int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned long long int val) = 0;

			/// Bind a boolean to parameter i.
			virtual void	bindParamB(unsigned int i, bool val) = 0;

			/// Bind a floating point number to parameter i.
			virtual void	bindParamF(unsigned int i, double val) = 0;
			/// Bind a floating point number to parameter i.
			virtual void	bindParamF(unsigned int i, float val) = 0;

			/// Bind a string to parameter i.
			virtual void	bindParamS(unsigned int i, const Glib::ustring &) = 0;

			/// Bind a duration to parameter i.
			virtual void	bindParamT(unsigned int i, const boost::posix_time::time_duration &) = 0;
			/// Bind a date time to parameter i.
			virtual void	bindParamT(unsigned int i, const boost::posix_time::ptime &) = 0;

			/// Bind null to parameter i.
			virtual void	bindNull(unsigned int i) = 0;

			/// The SQL statement.
			const std::string sql;
	};
}

#endif

