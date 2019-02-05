#ifndef DB_COMMAND_H
#define DB_COMMAND_H

#include "command_fwd.h"
#include <glibmm/ustring.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <visibility.h>
#include <factory.h>
#include <type_traits>
#include "dbTypes.h"
#include "error.h"

namespace DB {
	/// Exception thrown when binding a parameter of type the connector doesn't support.
	class DLL_PUBLIC ParameterTypeNotSupported : public Error {
		public:
			ParameterTypeNotSupported();
	};

	/// Exception thrown when binding a parameter out of range of those defined in the command.
	class DLL_PUBLIC ParameterOutOfRange : public Error {
		public:
			ParameterOutOfRange();
	};

	/// Represents the basic options that can be passed when creating new commands.
	class DLL_PUBLIC CommandOptions {
		public:
			CommandOptions() = default;
			/// Constructor which populates the hash value only.
			CommandOptions(std::size_t hash, const CommandOptionsMap & = CommandOptionsMap());
			virtual ~CommandOptions() = default;

			/// An (optional) hash of the SQL statement.
			std::optional<std::size_t> hash;

		protected:
			/// Helper function to extract values from a CommandOptionsMap
			template<typename X, typename Y>
			static X get(const CommandOptionsMap & map, const Y & key, const X & def)
			{
				if (auto i = map.find(key); i != map.end()) {
					if constexpr (std::is_convertible<CommandOptionsMap::mapped_type, X>::value) {
						return i->second;
					}
					else {
						return boost::lexical_cast<X>(i->second);
					}
				}
				return def;
			}
			/// Helper function to test if a value is set in a CommandOptionsMap
			static bool isSet(const CommandOptionsMap & map, const std::string & key);
	};

	/// Represents the basics of any command to be executed against a database.
	class DLL_PUBLIC Command {
		public:
			/// Creates a new command from the given SQL.
			Command(const std::string & sql);
			virtual ~Command() = 0;

			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, long val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, long long val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned long int val) = 0;
			/// Bind an integer to parameter i.
			virtual void	bindParamI(unsigned int i, unsigned long long int val) = 0;

			/// Bind a boolean to parameter i.
			virtual void	bindParamB(unsigned int i, bool val) = 0;

			/// Bind a floating point number to parameter i.
			virtual void	bindParamF(unsigned int i, double val) = 0;
			/// Bind a floating point number to parameter i.
			virtual void	bindParamF(unsigned int i, float val) = 0;

			/// Bind a string to parameter i.
			virtual void	bindParamS(unsigned int i, const Glib::ustring &) = 0;
			/// Bind a string_view to parameter i.
			virtual void	bindParamS(unsigned int i, const std::string_view &) = 0;
			/// Bind a string to parameter i (wraps string_view).
			inline void bindParamS(unsigned int i, const std::string & v)
			{
				bindParamS(i, std::string_view(v));
			}

			/// Bind a duration to parameter i.
			virtual void	bindParamT(unsigned int i, const boost::posix_time::time_duration &) = 0;
			/// Bind a date time to parameter i.
			virtual void	bindParamT(unsigned int i, const boost::posix_time::ptime &) = 0;

			/// Bind a BLOB to parameter i.
			virtual void	bindParamBLOB(unsigned int i, const Blob &);

			/// Bind null to parameter i.
			virtual void	bindNull(unsigned int i) = 0;

			/// The SQL statement.
			const std::string sql;

#define OPTWRAPPER(func) \
			template<typename O> \
			inline auto \
			func(unsigned int i, const O & o) -> typename std::enable_if< \
					std::is_constructible<bool, const O &>::value \
					&& !std::is_void<decltype(*o)>::value \
					>::type\
			{ \
				bool nn(o); \
				if (nn) \
					func(i, *o); \
				else \
					bindNull(i); \
			}
			/// @cond
			OPTWRAPPER(bindParamI);
			OPTWRAPPER(bindParamF);
			OPTWRAPPER(bindParamS);
			OPTWRAPPER(bindParamB);
			OPTWRAPPER(bindParamT);
			/// @endcond
#undef OPTWRAPPER
			/// Bind a (possibly null) c-string to parameter i.
			void bindParamS(unsigned int, const char * const);
			/// Bind a (possibly null) c-string to parameter i.
			void bindParamS(unsigned int, char * const);
	};
	typedef AdHoc::Factory<CommandOptions, std::size_t, const CommandOptionsMap &> CommandOptionsFactory;
}

#endif

