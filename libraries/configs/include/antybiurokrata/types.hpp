/**
 * @file types.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Here are stored most basic checks / definitions
 * 
 * @copyright Copyright (c) 2021
 * 
*/

#pragma once

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/libraries/patterns/seiralizer.hpp>

// STL
#include <locale>
#include <concepts>
#include <stdexcept>

namespace core
{
	/** @brief unifies string type in whole project */
	using str = std::string;

	/** @brief unifies char type in whole project */
	using char_t = typename str::value_type;

	/** @brief unifies string view type in whole project */
	using str_v = std::string_view;

	/** @brief unifies string type for utf-8 */
	using u16str = std::u16string;

	/** @brief unifies char type for utf-8 */
	using u16char_t = typename u16str::value_type;

	/** @brief unifies string view type for utf-8 */
	using u16str_v = std::u16string_view;

	/** @brief locale string for polish localization */
	constexpr str_v polish_locale{ "pl_PL.UTF-8" };

	struct u16str_serial;
	struct u16str_deserial;

	/** @brief handy type for wide type */
	template<auto X> using u16ser = patterns::serial::ser<X, u16str, u16str_serial, u16str_deserial>;

	/** @brief handy type for wide view type. view is read-only, so it's impossible to deserialize*/
	template<auto X> using u16vser = u16ser<X>;

	/**
	 * @brief returns the conversion engine object
	 * 
	 * @return std::wstring_convert
	 */
	inline auto get_conversion_engine()
	{
		/** 
		 * @brief standard library abomination, to bypass deleter 
		 * @link https://en.cppreference.com/w/cpp/locale/codecvt#Example
		*/
		using codecvt_t = std::codecvt<u16char_t, char_t, std::mbstate_t>;
		struct deletable_codecvt : public codecvt_t { ~deletable_codecvt() {} };
		return std::wstring_convert<deletable_codecvt, u16char_t>{"Error", u"Error"};
	}

	struct u16str_serial
	{
		template<typename stream_type>
		u16str_serial(stream_type& os, const u16str_v& view)
		{
			using patterns::serial::delimiter;
			os << view.size() << delimiter;
			for(const auto c : view) os << static_cast<int>(c) << delimiter;
		}
	};

	struct u16str_deserial
	{
		template<typename stream_type>
		u16str_deserial(stream_type& is, u16str& out)
		{
			using patterns::serial::delimiter;
			size_t size;
			is >> size;
			if(size == 0) return;
			else out.reserve(size);
			for(size_t i = 0; i < size; ++i)
			{
				int c;
				is >> c;
				out += static_cast<u16char_t>(c);
			}
		}
	};

	/**
	 * @brief contains basic defninitions and tools for throwing exceptions
	 */
	namespace exceptions
	{
		/**
		 * @brief verifies is type can be represented with string
		 * 
		 * @tparam T type to check
		 */
		template <typename T>
		concept stringizable = requires { std::is_convertible_v<T, str>; };

		/**
		 * @brief basic exception, as recommended it's derived freom std::exception
		 * 
		 * @tparam T provides logger for all child exception
		 */
		template <typename T>
		struct exception_base : public std::exception, Log<exception_base<T>>
		{
			using Log<exception_base<T>>::get_logger;

			/** stores message */
			const str ___what; 

			template<typename MsgType>
			explicit exception_base(MsgType msg) : ___what{ msg } {}

			virtual const char* what() const noexcept override { return this->___what.c_str(); }
			// virtual const str& what() const noexcept { return this->___what; }
			virtual str_v what_v() const noexcept { return this->___what; }
		};

		/**
		 * @brief simplest exception
		 */
		struct exception : public exception_base<exception> {};

		/**
		 * @brief default exception, that is raised on failed check
		 */
		struct assert_exception : public exception_base<assert_exception> { using exception_base<assert_exception>::exception_base; };

		/**
		 * @brief same as exception, but additionally prints reason to stdout, usefull, if extended log is required
		 */
		struct tee_exception : public exception_base<tee_exception>
		{
			template <stringizable U>
			explicit tee_exception(const U &msg) : exception_base{msg} { get_logger().error(what()); }
		};

		/**
		 * @brief verifies is given type has minimum requirements for project-wide exceptions
		 * 
		 * @tparam T type to check
		 */
		template <typename T>
		concept supported_exception = requires
		{
			std::is_base_of_v<std::exception, T>;
		};

		/**
		 * @brief alternative to asertion
		 * 
		 * @tparam _ExceptionType exception to throw if check failed
		 */
		template <supported_exception _ExceptionType = exception, bool __log_pass = false>
		struct require : Log<require<_ExceptionType>>
		{
			using Log<require<_ExceptionType>>::get_logger;
			/**
			 * @brief Construct a new require object, which is also checker
			 * 
			 * @tparam MsgType deducted type, of message
			 * @tparam ExceptionArgs variadic type of additional argument that are passed to exception constructor
			 * @param _check value to be checked
			 * @param msg message to pass to exception
			 * @param argv optional exception arguments
			 */
			template <typename MsgType, typename... ExceptionArgs>
			explicit require(const bool _check, const MsgType &msg = "no message provided", ExceptionArgs &&...argv)
			{
				if (_check) [[likely]] // be optimist :)
				{
					if constexpr (__log_pass)
						get_logger() << "passed: `" << msg << "`\n";// << logger::endl;
				}
				else [[unlikely]]
				{
					get_logger().error() << "Failed on check: "  << msg << logger::endl;
					get_logger().print_stacktrace();
					throw _ExceptionType(msg, std::forward<ExceptionArgs>(argv)...);
				}
			}
		};

	};

	/**
	 * @brief alias for assertion project-wide [ c(ustom) assert ]
	 * 
	 * @tparam Ex_t exception to throw, by default assert_exception
	 */
	template<typename Ex_t = typename exceptions::assert_exception>
	using cassert = typename exceptions::require<Ex_t>;
	
	/** [ d(efault) asssert ] */
	using dassert = cassert<>; 

	/** [ v(erbouse) d(efault) asssert ] */
	using vdassert = exceptions::require<typename exceptions::assert_exception, true>; 
} // namespace core