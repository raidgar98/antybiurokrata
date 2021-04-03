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

// STL
#include <concepts>
#include <stdexcept>

namespace core
{
	/**
	 * @brief unifies string type in whole project
	*/
	using str = std::string;

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

			virtual const char *what() const noexcept override
			{
				return (get_class_name<T>() + ___what).c_str();
			}
		};

		/**
		 * @brief simplest exception
		 */
		struct exception : public exception_base<exception> {};

		/**
		 * @brief default exception, that is raised on failed check
		 */
		struct assert_exception : public exception_base<assert_exception> {};

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
		 * @brief this is just holder for __log_pass
		 */
		struct ____require_base
		{
		protected:

			/** if set to true logs passed checks */
			static constexpr bool __log_pass{false}; 
		};

		/**
		 * @brief alternative to asertion
		 * 
		 * @tparam _ExceptionType exception to throw if check failed
		 */
		template <supported_exception _ExceptionType = exception>
		struct require : Log<require<_ExceptionType>>, ____require_base
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
						get_logger() << "passed: `" << msg << '`' << logger::endl;
				}
				else [[unlikely]]
				{
					get_logger().error("Failed on check");
					get_logger() << msg << logger::endl;
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
} // namespace core