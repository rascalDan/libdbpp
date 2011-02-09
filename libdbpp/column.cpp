#include "column.h"

DB::Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n)
{
}

DB::Column::~Column()
{
}


