#ifndef DB_CONNECTION_FWD_H
#define DB_CONNECTION_FWD_H

#include <memory>

namespace DB {
	class Connection;
	typedef std::shared_ptr<Connection> ConnectionPtr;
	typedef std::shared_ptr<const Connection> ConnectionCPtr;
}

#endif

