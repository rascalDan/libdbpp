#include "error.h"
#include <time.h>

DB::ConnectionError::ConnectionError() :
	FailureTime(time(NULL))
{
}

DB::ConnectionError::ConnectionError(time_t t) :
	FailureTime(t)
{
}

