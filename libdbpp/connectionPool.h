#ifndef DB_CONNECTIONPOOL_H
#define DB_CONNECTIONPOOL_H

#include <resourcePool.h>
#include <visibility.h>
#include <memory>
#include "connection.h"

namespace DB {
	/// Specialisation of AdHoc::ResourcePool for database connections.
	class DLL_PUBLIC ConnectionPool : public AdHoc::ResourcePool<Connection> {
		public:
			/// Create a new connection pool.
			/// @param max Maximum number of concurrent database connections.
			/// @param keep Number of connections to keep open after use.
			/// @param type Database connection factory name.
			/// @param connectionString Connection string to pass to the connection factory.
			ConnectionPool(unsigned int max, unsigned int keep, const std::string & type, const std::string & connectionString);

		protected:
			/// Create a new connection.
			ConnectionPtr createResource() const override;
			/// Ping a connection.
			void returnTestResource(const ConnectionCPtr &) const override;
			/// Ping a connection.
			void testResource(const ConnectionCPtr &) const override;

		private:
			const ConnectionFactoryCPtr factory;
			const std::string connectionString;
	};
	typedef std::shared_ptr<ConnectionPool> ConnectionPoolPtr;
}

#endif

