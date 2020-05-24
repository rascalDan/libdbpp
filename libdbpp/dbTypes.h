#ifndef DB_TYPES_H
#define DB_TYPES_H

#include <visibility.h>
#include <cstdlib>
#include <vector>

namespace DB {
	/// Wrapper class for reference an existing block of binary data.
	class DLL_PUBLIC Blob {
		public:
			/// Construct a default blob pointing to no data.
			Blob() = default;
			/// Construct a reference using C-style pointer and length.
			Blob(const void * data, size_t len);
			/// Construct a reference using C++ template pointer to an object.
			template<typename T>
			explicit Blob(const T * t) :
				data(t),
				len(sizeof(T))
			{
			}
			/// Construct a reference using C++ vector pointer to a collection of objects.
			template<typename T>
			// NOLINTNEXTLINE(hicpp-explicit-conversions)
			Blob(const std::vector<T> & v) :
				data(&v.front()),
				len(sizeof(T) * v.size())
			{
			}

			/// Byte-wise equality operation.
			bool operator==(const DB::Blob b) const;

			/// The beginning of the binary data.
			const void * data { nullptr };
			/// The length of the binary data.
			size_t len { 0 };
	};
}

#endif

