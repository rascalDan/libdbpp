#include "selectcommand.h"
#include "error.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <buffer.h>

namespace DB {
	ColumnIndexOutOfRange::ColumnIndexOutOfRange(unsigned int n) : colNo(n) { }

	std::string
	ColumnIndexOutOfRange::message() const throw()
	{
		return stringf("Column (%u) index out of range", colNo);
	}

	ColumnDoesNotExist::ColumnDoesNotExist(const Glib::ustring & n) : colName(n) { }

	std::string
	ColumnDoesNotExist::message() const throw()
	{
		return stringf("Column (%s) does not exist", colName.c_str());
	}
};

DB::SelectCommand::SelectCommand(const std::string & sql) :
	DB::Command(sql),
	columns(new Columns)
{
}

DB::SelectCommand::~SelectCommand()
{
	delete columns;
}

const DB::Column&
DB::SelectCommand::operator[](unsigned int n) const
{
	if (n < columns->size()) {
		return **columns->get<0>().find(n);
	}
	throw ColumnIndexOutOfRange(n);
}

const DB::Column&
DB::SelectCommand::operator[](const Glib::ustring & n) const
{
	typedef Columns::nth_index<1>::type CbyName;
	CbyName::iterator i = columns->get<1>().find(n);
	if (i != columns->get<1>().end()) {
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
	return columns->size();
}

DB::ColumnPtr
DB::SelectCommand::insertColumn(ColumnPtr col)
{
	return *columns->insert(col).first;
}

