#include "dbTypes.h"
#include <memory.h>

DB::Blob::Blob(const void * d, size_t l) : data(d), len(l) { }

bool
DB::Blob::operator==(const DB::Blob b) const
{
	return this->len == b.len && memcmp(this->data, b.data, this->len) == 0;
}
