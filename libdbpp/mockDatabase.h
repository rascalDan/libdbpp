#ifndef MOCKDATABASE_H
#define MOCKDATABASE_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include "connection.h"
#include <visibility.h>

namespace DB {

class DLL_PUBLIC MockDatabase {
	public:
		MockDatabase(const std::string & mockName);
		virtual ~MockDatabase();

		virtual DB::Connection * openConnection() const = 0;

	protected:
		virtual void CreateNewDatabase() const = 0;
		virtual void PlaySchemaScripts(const std::vector<boost::filesystem::path> & ss) const;
		virtual void PlaySchemaScript(DB::Connection *, const boost::filesystem::path & s) const;
		virtual void DropDatabase() const = 0;
		virtual void UpdateStatusTable(DB::Connection *, const boost::filesystem::path &) const;

		const std::string mockName;
		static unsigned int mocked;

	private:
		void CreateStatusTable(DB::Connection *) const;
		void DropStatusTable(DB::Connection *) const;
};

class DLL_PUBLIC MockServerDatabase : public MockDatabase {
	public:
		MockServerDatabase(const std::string & masterdb, const std::string & name, const std::string & type);
		virtual ~MockServerDatabase();

	protected:
		virtual void CreateNewDatabase() const override;
		virtual void DropDatabase() const override;

		DB::Connection * master;
		const std::string testDbName;
};

}

#endif

