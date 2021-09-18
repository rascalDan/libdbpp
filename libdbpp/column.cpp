#include "column.h"
#include <compileTimeFormatter.h>
#include <cxxabi.h>
#include <exception>
#include <memory>

namespace DB {
	Column::Column(const Glib::ustring & n, unsigned int i) : colNo(i), name(n.collate_key()) { }

	static std::string
	demangle(const char * const mangled)
	{
		std::unique_ptr<char, decltype(&free)> r(abi::__cxa_demangle(mangled, nullptr, nullptr, nullptr), &free);
		return &*r;
	}

	InvalidConversion::InvalidConversion(const char * const f, const char * const t) :
		from(demangle(f)), to(demangle(t))
	{
	}

	AdHocFormatter(InvalidConversionMsg, "Invalid conversion from column type (%?) to value type (%?)");
	std::string
	InvalidConversion::message() const noexcept
	{
		return InvalidConversionMsg::get(from, to);
	}

	UnexpectedNullValue::UnexpectedNullValue(const char * const t) : to(t) { }

	AdHocFormatter(UnexpectedNullValueMsg, "Unexpected null value in column expecting type (%?)");
	std::string
	UnexpectedNullValue::message() const noexcept
	{
		return InvalidConversionMsg::get(to, to);
	}

	void
	DB::HandleField::blob(const DB::Blob &)
	{
		throw DB::ColumnTypeNotSupported();
	}
}
