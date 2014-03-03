#ifndef DB_COMMAND_H
#define DB_COMMAND_H

#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace DB {
	class Command {
		public:
			Command(const std::string & sql);
			virtual ~Command() = 0;

			virtual void	bindParamI(unsigned int i, int val) = 0;
			virtual void	bindParamI(unsigned int i, long val) = 0;
			virtual void	bindParamI(unsigned int i, long long val) = 0;
			virtual void	bindParamI(unsigned int i, unsigned int val) = 0;
			virtual void	bindParamI(unsigned int i, unsigned long int val) = 0;
			virtual void	bindParamI(unsigned int i, unsigned long long int val) = 0;

			virtual void	bindParamF(unsigned int i, double val) = 0;
			virtual void	bindParamF(unsigned int i, float val) = 0;

			virtual void	bindParamS(unsigned int i, const Glib::ustring &) = 0;

			virtual void	bindParamT(unsigned int i, const boost::posix_time::time_duration &) = 0;
			virtual void	bindParamT(unsigned int i, const boost::posix_time::ptime &) = 0;

			virtual void	bindNull(unsigned int i) = 0;

			const std::string sql;
	};
}

#endif

