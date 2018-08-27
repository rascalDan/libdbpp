#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <stdlib.h>
#include <exception.h>
#include <visibility.h>

namespace DB {
	/// Base class for database errors.
	class DLL_PUBLIC Error : public virtual std::exception {
	};

	/// Exception thrown when attempting to bulk upload with a connector that doesn't support it.
	class DLL_PUBLIC BulkUploadNotSupported : public Error {
		public:
			BulkUploadNotSupported();
	};

	/// Exception thrown when a query returns an unsupported column type.
	class DLL_PUBLIC ColumnTypeNotSupported : public Error {
		public:
			ColumnTypeNotSupported();
	};

	/// Exception thrown on an attempt to convert betweem incompatible types.
	class DLL_PUBLIC InvalidConversion : public AdHoc::Exception<Error> {
		public:
			/// Create a new InvalidConversion exception with the names of the conversion types.
			/// @param from Source type
			/// @param to Destination type
			InvalidConversion(const char * const from, const char * const to);

		private:
			std::string message() const throw() override;
			const char * from;
			const char * to;
	};

	class DLL_PUBLIC UnexpectedNullValue : public AdHoc::Exception<Error> {
		public:	
			UnexpectedNullValue(const char * const from);

		private:
			std::string message() const throw() override;
			const char * to;
	};
}

#endif
