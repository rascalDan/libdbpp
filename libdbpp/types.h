#ifndef DB_TYPES_H
#define DB_TYPES_H

#include <visibility.h>
#include <stdlib.h>
#include <vector>

namespace DB {
	/// Wrapper class for reference an existing block of binary data.
	class DLL_PUBLIC Blob {
		public:
			/// Construct a reference using C-style pointer and length.
			Blob(const void * data, size_t len);
			/// Construct a reference using C++ template pointer to an object.
			template<typename T>
			Blob(const T * t) :
				data(t),
				len(sizeof(T))
			{
			}
			/// Construct a reference using C++ vector pointer to a collection of objects.
			template<typename T>
			Blob(const std::vector<T> & v) :
				data(&v.front()),
				len(sizeof(T) * v.size())
			{
			}

			/// The beginning of the binary data.
			const void * data;
			/// The length of the binary data.
			size_t len;
	};
}

#endif

