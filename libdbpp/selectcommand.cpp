#include "selectcommand.h"
#include "error.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <compileTimeFormatter.h>

namespace DB {
	ColumnIndexOutOfRange::ColumnIndexOutOfRange(unsigned int n) : colNo(n) { }

	AdHocFormatter(ColumnIndexOutOfRangeMsg, "Column (%?) index out of range");
	std::string
	ColumnIndexOutOfRange::message() const noexcept
	{
		return ColumnIndexOutOfRangeMsg::get(colNo);
	}

	ColumnDoesNotExist::ColumnDoesNotExist(Glib::ustring n) : colName(std::move(n)) { }

	AdHocFormatter(ColumnDoesNotExistMsg, "Column (%?) does not exist");
	std::string
	ColumnDoesNotExist::message() const noexcept
	{
		return ColumnDoesNotExistMsg::get(colName);
	}

	using ColumnsBase = boost::multi_index_container<ColumnPtr, boost::multi_index::indexed_by<
		boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const unsigned int, &DB::Column::colNo>>,
		boost::multi_index::ordered_unique<boost::multi_index::member<DB::Column, const std::string, &DB::Column::name>>
						>>;
	class SelectCommand::Columns : public ColumnsBase { };
};

DB::SelectCommand::SelectCommand(const std::string & sql) :
	DB::Command(sql),
	columns(std::make_unique<Columns>())
{
}

DB::SelectCommand::~SelectCommand() = default;

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
	if (auto i = columns->get<1>().find(n.collate_key());
			i != columns->get<1>().end()) {
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

const DB::ColumnPtr &
DB::SelectCommand::insertColumn(ColumnPtr col)
{
	return *columns->insert(std::move(col)).first;
}

DB::RowBase::RowBase(SelectCommand * s) :
	sel(s)
{
}

const DB::Column&
DB::RowBase::operator[](const Glib::ustring & n) const
{
	return sel->operator[](n);
}

const DB::Column&
DB::RowBase::operator[](unsigned int col) const
{
	return sel->operator[](col);
}

