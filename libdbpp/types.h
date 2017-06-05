#ifndef DB_TYPES_H
#define DB_TYPES_H

#include <visibility.h>
#include <stdlib.h>
#include <vector>

namespace DB {
	class DLL_PUBLIC Blob {
		public:
			Blob(const void * data, size_t len);
			template<typename T>
			Blob(const T * t) :
				data(t),
				len(sizeof(T))
			{
			}
			template<typename T>
			Blob(const std::vector<T> & v) :
				data(&v.front()),
				len(sizeof(T) * v.size())
			{
			}

			const void * data;
			size_t len;
	};
}

#endif

