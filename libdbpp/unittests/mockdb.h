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

		void execute(const std::string & sql) override;
		DB::SelectCommand * newSelectCommand(const std::string &) override;
		DB::ModifyCommand * newModifyCommand(const std::string &) override;

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

