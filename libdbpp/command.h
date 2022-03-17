#ifndef DB_COMMAND_H
#define DB_COMMAND_H

#include "command_fwd.h"
#include "error.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <c++11Helpers.h>
#include <cstddef>
#include <factory.h> // IWYU pragma: keep
#include <optional>
#include <string>
#include <string_view>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop
#include <type_traits>
#include <visibility.h>
// IWYU pragma: no_include "factory.impl.h"

namespace DB {
	class Blob;
	/// Exception thrown when binding a parameter of type the connector doesn't support.
	class DLL_PUBLIC ParameterTypeNotSupported : public Error {
	};

	/// Exception thrown when binding a parameter out of range of those defined in the command.
	class DLL_PUBLIC ParameterOutOfRange : public Error {
	};

	/// Represents the basic options that can be passed when creating new commands.
	class DLL_PUBLIC CommandOptions {
	public:
		CommandOptions() = default;
		/// Constructor which populates the hash value only.
		explicit CommandOptions(std::size_t hash, const CommandOptionsMap & = CommandOptionsMap());
		virtual ~CommandOptions() = default;
		/// Standard special members
		SPECIAL_MEMBERS_DEFAULT(CommandOptions);

		/// An (optional) hash of the SQL statement.
		std::optional<std::size_t> hash;

	protected:
		/// Helper function to extract values from a CommandOptionsMap
		template<typename X, typename Y>
		static X
		get(const CommandOptionsMap & map, const Y & key, const X & def)
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
		explicit Command(std::string sql);
		virtual ~Command() = default;

		/// Standard special members
		SPECIAL_MEMBERS_COPY_RO(Command);

		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, int val) = 0;
		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, long val) = 0;
		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, long long val) = 0;
		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, unsigned int val) = 0;
		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, unsigned long int val) = 0;
		/// Bind an integer to parameter i.
		virtual void bindParamI(unsigned int i, unsigned long long int val) = 0;

		/// Bind a boolean to parameter i.
		virtual void bindParamB(unsigned int i, bool val) = 0;

		/// Bind a floating point number to parameter i.
		virtual void bindParamF(unsigned int i, double val) = 0;
		/// Bind a floating point number to parameter i.
		virtual void bindParamF(unsigned int i, float val) = 0;

		/// Bind a string to parameter i.
		virtual void bindParamS(unsigned int i, const Glib::ustring &) = 0;
		/// Bind a string_view to parameter i.
		virtual void bindParamS(unsigned int i, const std::string_view) = 0;
		/// Bind a string to parameter i (wraps string_view).
		inline void
		bindParamS(unsigned int i, const std::string & v)
		{
			bindParamS(i, std::string_view(v));
		}

		/// Bind a duration to parameter i.
		virtual void bindParamT(unsigned int i, const boost::posix_time::time_duration) = 0;
		/// Bind a date time to parameter i.
		virtual void bindParamT(unsigned int i, const boost::posix_time::ptime) = 0;

		/// Bind a BLOB to parameter i.
		virtual void bindParamBLOB(unsigned int i, const Blob &);

		/// Bind null to parameter i.
		virtual void bindNull(unsigned int i) = 0;

		/// The SQL statement.
		const std::string sql;

		/// Bind a parameter by type based on C++ traits to parameter i.
		template<typename O>
		inline void
		bindParam(unsigned int i, const O & o)
		{
			if constexpr (std::is_null_pointer<O>::value || std::is_same<O, std::nullopt_t>::value) {
				bindNull(i);
			}
			else if constexpr (std::is_same<O, bool>::value) {
				bindParamB(i, o);
			}
			else if constexpr (std::is_floating_point<O>::value) {
				bindParamF(i, o);
			}
			else if constexpr (std::is_same<O, boost::posix_time::time_duration>::value
					|| std::is_same<O, boost::posix_time::ptime>::value) {
				bindParamT(i, o);
			}
			else if constexpr (std::is_same<O, Blob>::value || std::is_convertible<O, Blob>::value) {
				bindParamBLOB(i, o);
			}
			else if constexpr (std::is_integral<O>::value && !std::is_pointer<O>::value) {
				bindParamI(i, o);
			}
			else if constexpr (std::is_convertible<O, std::string_view>::value && std::is_pointer<O>::value) {
				if (o) {
					bindParamS(i, o);
				}
				else {
					bindNull(i);
				}
			}
			else if constexpr (std::is_same<O, Glib::ustring>::value
					|| std::is_convertible<O, std::string_view>::value) {
				// NOLINTNEXTLINE(hicpp-no-array-decay)
				bindParamS(i, o);
			}
			else if constexpr (std::is_constructible<bool, const O &>::value) {
				if (o) {
					bindParam(i, *o);
				}
				else {
					bindNull(i);
				}
			}
			else {
				static_assert(std::is_void_v<O>, "No suitable trait");
			}
		}

#define OPTWRAPPER(func) \
	template<typename O> \
	inline auto func(unsigned int i, const O & o) \
			->typename std::enable_if<std::is_constructible_v<bool, const O &> && !std::is_void_v<decltype(*o)>>::type \
	{ \
		bool nn(o); \
		if (nn) \
			func(i, *o); \
		else \
			bindNull(i); \
	}
		/// @cond
		OPTWRAPPER(bindParamI)
		OPTWRAPPER(bindParamF)
		// NOLINTNEXTLINE(hicpp-no-array-decay)
		OPTWRAPPER(bindParamS)
		OPTWRAPPER(bindParamB)
		OPTWRAPPER(bindParamT)
		/// @endcond
#undef OPTWRAPPER
		/// Bind a (possibly null) c-string to parameter i.
		void bindParamS(unsigned int, const char * const);
		/// Bind a (possibly null) c-string to parameter i.
		void bindParamS(unsigned int, char * const);
	};
	using CommandOptionsFactory = AdHoc::Factory<CommandOptions, std::size_t, const CommandOptionsMap &>;
}

#endif
