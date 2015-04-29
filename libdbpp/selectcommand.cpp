#include "selectcommand.h"
#include "error.h"

namespace DB {
	class ColumnIndexOutOfRange : public Error {
		public:
			ColumnIndexOutOfRange(unsigned int n) : colNo(n) { }

			const unsigned int colNo;
	};
	class ColumnDoesNotExist : public Error {
		public:
			ColumnDoesNotExist(const Glib::ustring & n) : colName(n) { }

			const Glib::ustring colName;
	};
};

DB::SelectCommand::SelectCommand(const std::string & sql) :
	DB::Command(sql)
{
}

DB::SelectCommand::~SelectCommand()
{
}

const DB::Column&
DB::SelectCommand::operator[](unsigned int n) const
{
	if (n < columns.size()) {
		return **columns.get<0>().find(n);
	}
	throw ColumnIndexOutOfRange(n);
}

const DB::Column&
DB::SelectCommand::operator[](const Glib::ustring & n) const
{
	typedef Columns::nth_index<1>::type CbyName;
	CbyName::iterator i = columns.get<1>().find(n);
	if (i != columns.get<1>().end()) {
		return **i;
	}
	throw ColumnDoesNotExist(n);
}

unsigned int
DB::SelectCommand::getOrdinal(const Glib::ustring & n) const
{
	return operator[](n).colNo;
}

unsigned int
DB::SelectCommand::columnCount() const
{
	return columns.size();
}

