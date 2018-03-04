#ifndef DB_CONNECTION_FWD_H
#define DB_CONNECTION_FWD_H

#include <boost/shared_ptr.hpp>

namespace DB {
	class Connection;
	typedef boost::shared_ptr<Connection> ConnectionPtr;
}

#endif

