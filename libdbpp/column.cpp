#include "column.h"
#include <exception>
#include <compileTimeFormatter.h>

namespace DB {
Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n.collate_key())
{
}

Column::~Column()
{
}

InvalidConversion::InvalidConversion(const char * const f, const char * const t) : from(f), to(t) { }

AdHocFormatter(InvalidConversionMsg, "Invalid conversion from column type (%?) to value type (%?)");
std::string
InvalidConversion::message() const throw()
{
	return InvalidConversionMsg::get(from, to);
}

UnexpectedNullValue::UnexpectedNullValue(const char * const t) : to(t) { }

AdHocFormatter(UnexpectedNullValueMsg, "Unexpected null value in column expecting type (%?)");
std::string
UnexpectedNullValue::message() const throw()
{
	return InvalidConversionMsg::get(to, to);
}

void
DB::HandleField::blob(const DB::Blob &)
{
	throw DB::ColumnTypeNotSupported();
}

}

