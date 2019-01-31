#ifndef DB_MOCKDATABASE_H
#define DB_MOCKDATABASE_H

#include <string>
#include <vector>
#include <filesystem>
#include "connection.h"
#include <visibility.h>

namespace DB {

/// MockDatabase creates, registers and destroys a database suitable for unit testing.
class DLL_PUBLIC MockDatabase : public AdHoc::AbstractPluginImplementation {
	public:
		/// Creates and registers a new database.
		virtual ~MockDatabase() = default;

		/// Open a connection to this database instance.
		virtual ConnectionPtr openConnection() const = 0;

		/// Open a connection to a named mock database.
		static ConnectionPtr openConnectionTo(const std::string &);

	protected:
		/// Implementation specific method to create a new database.
		virtual void CreateNewDatabase() const = 0;
		/// Execute ordered collection of scripts to setup mock database. e.g. schema.sql, sampleData.sql
		virtual void PlaySchemaScripts(const std::vector<std::filesystem::path> & ss) const;
		/// Execute a single setup script.
		virtual void PlaySchemaScript(DB::Connection *, const std::filesystem::path & s) const;
		/// Implementation specific method to drop a database.
		virtual void DropDatabase() const = 0;

		/// Internal counter of mocked databases (for unique name generation)
		static unsigned int mocked;
};

/// MockServerDatabase extends MockDatabase with the functionality to connect to a database service
/// and create your mock on there.
class DLL_PUBLIC MockServerDatabase : public MockDatabase {
	public:
		/// Create and register a new database.
		/// @param masterdb connection to server with permissions to create a new database.
		/// @param name the prefix to use when creating databases.
		/// @param type the database type.
		MockServerDatabase(const std::string & masterdb, const std::string & name, const std::string & type);
		virtual ~MockServerDatabase();

		/// Get the database instance name on the server.
		const std::string & databaseName() const;

	protected:
		virtual void CreateNewDatabase() const override;
		virtual void DropDatabase() const override;

		/// Connection to the master database.
		DB::ConnectionPtr master;
		/// The name of the database that was created on the server.
		const std::string testDbName;
};

/// Helper class for creating instances of mock databases
template<typename T>
class PluginMock {
	public:
		/// Create and register a new mock database.
		/// @param name the name of the mock database to register.
		/// @param s the collection of scripts to populate the mock database.
		/// @param args arguments to the mock database constructor.
		template<typename ... Args>
		PluginMock(const std::string & name, const std::initializer_list<std::filesystem::path> & s, const Args & ... args) :
			mockName(name)
		{
			AdHoc::PluginManager::getDefault()->create<MockDatabase, T>(mockName, __FILE__, __LINE__, args..., name, s);
		}
		~PluginMock()
		{
			AdHoc::PluginManager::getDefault()->remove<MockDatabase>(mockName);
		}
		/// Get the name of the mock database.
		const std::string & databaseName() const
		{
			return std::dynamic_pointer_cast<MockServerDatabase>(AdHoc::PluginManager::getDefault()->get<MockDatabase>(mockName)->implementation())->databaseName();
		}

		/// The name of this mocked database.
		const std::string mockName;
};

typedef AdHoc::Factory<MockDatabase, const std::string &, const std::string &, const std::vector<std::filesystem::path> &> MockDatabaseFactory;
}

#endif

