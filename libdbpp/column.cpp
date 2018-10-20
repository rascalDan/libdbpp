#include "column.h"
#include <exception>
#include <compileTimeFormatter.h>
#include <memory>
#include <cxxabi.h>

namespace DB {
Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n.collate_key())
{
}

Column::~Column()
{
}

static
std::string
demangle(const char * const mangled)
{
	std::unique_ptr<char, decltype(&free)> r(abi::__cxa_demangle(mangled, NULL, NULL, NULL), &free);
	return &*r;
};

InvalidConversion::InvalidConversion(const char * const f, const char * const t) : from(demangle(f)), to(demangle(t)) { }

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

