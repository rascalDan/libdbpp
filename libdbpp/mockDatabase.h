#ifndef DB_MOCKDATABASE_H
#define DB_MOCKDATABASE_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include "connection.h"
#include <visibility.h>

namespace DB {

/// MockDatabase creates, registers and destroys a database suitable for unit testing.
class DLL_PUBLIC MockDatabase : public AdHoc::AbstractPluginImplementation {
	public:
		/// Creates and registers a new database.
		/// @param mockName the name the database will register as.
		MockDatabase(const std::string & mockName);
		virtual ~MockDatabase();

		/// Open a connection to this database instance.
		virtual DB::Connection * openConnection() const = 0;

		/// Open a connection to a named mock database.
		static Connection * openConnectionTo(const std::string &);

	protected:
		/// Implementation specific method to create a new database.
		virtual void CreateNewDatabase() const = 0;
		/// Execute ordered collection of scripts to setup mock database. e.g. schema.sql, sampleData.sql
		virtual void PlaySchemaScripts(const std::vector<boost::filesystem::path> & ss) const;
		/// Execute a single setup script.
		virtual void PlaySchemaScript(DB::Connection *, const boost::filesystem::path & s) const;
		/// Implementation specific method to drop a database.
		virtual void DropDatabase() const = 0;
		/// Update the status table.
		virtual void UpdateStatusTable(DB::Connection *, const boost::filesystem::path &) const;

		/// The name of this mocked database.
		const std::string mockName;
		/// Internal counter of mocked databases (for unique name generation)
		static unsigned int mocked;

	private:
		void CreateStatusTable(DB::Connection *) const;
		void DropStatusTable(DB::Connection *) const;
};

/// MockServerDatabase extends MockDatabase with the functionality to connect to a database service
/// and create your mock on there.
class DLL_PUBLIC MockServerDatabase : public MockDatabase {
	public:
		/// Create and register a new database.
		/// @param masterdb connection to server with permissions to create a new database.
		/// @param name the name of the mock to register as.
		/// @param type the database type.
		MockServerDatabase(const std::string & masterdb, const std::string & name, const std::string & type);
		virtual ~MockServerDatabase();

		/// Get the database instance name on the server.
		const std::string & databaseName() const;

	protected:
		virtual void CreateNewDatabase() const override;
		virtual void DropDatabase() const override;

		/// Connection to the master database.
		DB::Connection * master;
		/// The name of the database that was created on the server.
		const std::string testDbName;
};

}

#endif

