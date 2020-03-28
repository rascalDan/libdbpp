#ifndef DB_CONNECTIONPOOL_H
#define DB_CONNECTIONPOOL_H

#include <resourcePool.h>
#include <visibility.h>
#include <memory>
#include "connection.h"

namespace DB {
	/// Specialisation of AdHoc::ResourcePool for database connections.
	class DLL_PUBLIC BasicConnectionPool : public AdHoc::ResourcePool<Connection> {
		public:
			/// Create a new connection pool.
			/// @param max Maximum number of concurrent database connections.
			/// @param keep Number of connections to keep open after use.
			BasicConnectionPool(unsigned int max, unsigned int keep);

		protected:
			/// Ping a connection.
			void returnTestResource(Connection const *) const override;
			/// Ping a connection.
			void testResource(Connection const *) const override;
	};

	/// Standard specialisation of AdHoc::ResourcePool for database connections given a type and connection string.
	class DLL_PUBLIC ConnectionPool : public BasicConnectionPool {
		public:
			/// Create a new connection pool.
			/// @param max Maximum number of concurrent database connections.
			/// @param keep Number of connections to keep open after use.
			/// @param type Database connection factory name.
			/// @param connectionString Connection string to pass to the connection factory.
			ConnectionPool(unsigned int max, unsigned int keep, const std::string & type, std::string connectionString);

		protected:
			/// Create a new connection.
			ConnectionPtr createResource() const override;

		private:
			const ConnectionFactoryCPtr factory;
			const std::string connectionString;
	};

	using ConnectionPoolPtr = std::shared_ptr<BasicConnectionPool>;
}

#endif

