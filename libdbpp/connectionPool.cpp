#include "connectionPool.h"
#include <factory.h>
#include <resourcePool.impl.h>
#include <utility>

template class AdHoc::ResourcePool<DB::Connection>;
template class AdHoc::ResourceHandle<DB::Connection>;

namespace DB {
	BasicConnectionPool::BasicConnectionPool(unsigned int m, unsigned int k) : ResourcePool<Connection>(m, k) { }

	ConnectionPool::ConnectionPool(unsigned int m, unsigned int k, const std::string & t, std::string cs) :
		BasicConnectionPool(m, k), factory(ConnectionFactory::get(t)), connectionString(std::move(cs))
	{
	}

	ConnectionPtr
	ConnectionPool::createResource() const
	{
		return factory->create(connectionString);
	}

	void
	BasicConnectionPool::returnTestResource(Connection const * c) const
	{
		c->finish();
	}

	void
	BasicConnectionPool::testResource(Connection const * c) const
	{
		c->ping();
	}
}
