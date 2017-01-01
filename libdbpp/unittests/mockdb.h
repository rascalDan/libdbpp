#ifndef DB_MOCKDB_H
#define DB_MOCKDB_H

#include "../connection.h"
#include "../mockDatabase.h"

class MockDb : public DB::Connection {
	public:
		MockDb(const std::string &);

		void beginTxInt() override;
		void commitTxInt() override;
		void rollbackTxInt() override;
		void ping() const override;
		DB::BulkDeleteStyle bulkDeleteStyle() const override;
		DB::BulkUpdateStyle bulkUpdateStyle() const override;

		void execute(const std::string & sql, const DB::CommandOptions *) override;
		DB::SelectCommand * newSelectCommand(const std::string &, const DB::CommandOptions *) override;
		DB::ModifyCommand * newModifyCommand(const std::string &, const DB::CommandOptions *) override;

		mutable std::vector<std::string> executed;
};

class MockMock : public DB::MockDatabase {
	public:
		MockMock(const std::string &, const std::string &, const std::vector<boost::filesystem::path> &);
		DB::Connection * openConnection() const override;
		void CreateNewDatabase() const override;
		void DropDatabase() const override;
};

#endif

