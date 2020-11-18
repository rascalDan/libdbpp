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

	void execute(const std::string & sql, const DB::CommandOptionsCPtr &) override;
	DB::SelectCommandPtr select(const std::string &, const DB::CommandOptionsCPtr &) override;
	DB::ModifyCommandPtr modify(const std::string &, const DB::CommandOptionsCPtr &) override;

	mutable std::vector<std::string> executed;
};

class MockMock : public DB::MockDatabase {
public:
	MockMock(const std::string &, const std::string &, const std::vector<std::filesystem::path> &);
	DB::ConnectionPtr openConnection() const override;
	void CreateNewDatabase() const override;
	void DropDatabase() const override;
};

#endif
