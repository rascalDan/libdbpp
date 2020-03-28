#ifndef DB_CONNECTION_FWD_H
#define DB_CONNECTION_FWD_H

#include <memory>

namespace DB {
	class Connection;
	using ConnectionPtr = std::shared_ptr<Connection>;
	using ConnectionCPtr = std::shared_ptr<const Connection>;
}

#endif

