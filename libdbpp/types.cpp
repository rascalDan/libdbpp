#include "types.h"

DB::Blob::Blob() :
	data(nullptr),
	len(0)
{
}

DB::Blob::Blob(const void * d, size_t l) :
	data(d),
	len(l)
{
}

