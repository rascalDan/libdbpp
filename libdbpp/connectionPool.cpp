#include "connectionPool.h"
#include <resourcePool.impl.h>

template class AdHoc::ResourcePool<DB::Connection>;
template class AdHoc::ResourceHandle<DB::Connection>;

namespace DB {
	ConnectionPool::ConnectionPool(unsigned int m, unsigned int k, const std::string & t, const std::string & cs) :
		ResourcePool<Connection>(m, k),
		factory(ConnectionFactory::get(t)),
		connectionString(cs)
	{
	}

	ConnectionPtr
	ConnectionPool::createResource() const
	{
		return factory->create(connectionString);
	}

	void
	ConnectionPool::returnTestResource(const ConnectionCPtr & c) const
	{
		c->finish();
	}

	void
	ConnectionPool::testResource(const ConnectionCPtr & c) const
	{
		c->ping();
	}
}

