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
#include <memory>
#include <locale>
#include <concepts>
#include <stdexcept>


// Boost
#include <boost/stacktrace.hpp>

/**
 * @brief base namespace for whole program
 */
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
}	 // namespace core

// literals
core::u16str operator"" _u16(const char* str, const size_t);
core::u16str operator"" _u16(const char16_t* str, const size_t s);
core::str operator"" _u8(const char16_t* str, const size_t);
core::str operator"" _u8(const char* c_str, const size_t s);

namespace core
{
	/** @brief locale string for polish localization */
	constexpr str_v polish_locale{"pl_PL.UTF-8"};

	/** @brief polish locale object */
	inline std::locale plPL()
	{
		struct no_separator : std::numpunct<char>
		{
			virtual char do_thousands_sep() const override { return '\0'; }
			virtual str do_grouping() const override { return ""; }
		};
		static const std::locale numbers_loc{std::locale{polish_locale.data()}, new no_separator};

		return numbers_loc;
	};

	struct u16str_serial;
	struct u16str_deserial;

	/** @brief handy type for wide type */
	// template<auto X> using u16ser = patterns::serial::ser<X, u16str, u16str_serial, u16str_deserial, u16str_pretty_serial>;

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
		struct deletable_codecvt : public codecvt_t
		{
			~deletable_codecvt() {}
		};
		return std::wstring_convert<deletable_codecvt, u16char_t>{"Error", u"Error"};
	}

	/**
	 * @brief contains basic defninitions and tools for throwing exceptions
	 */
	namespace exceptions
	{
		/**
		 * @brief basic exception, as recommended it's derived freom std::exception
		 * 
		 * @tparam T provides logger for all child exception
		 * @tparam MsgType message type, ex: core::str, core::u16str
		 */
		template<typename T, typename MsgType>
		struct exception_base : /* public std::exception,*/ Log<exception_base<T, MsgType>>
		{
			using Log<exception_base<T, MsgType>>::get_logger;

			/** stores message */
			const MsgType _what;

			template<typename Any>
			requires std::is_convertible_v<Any, MsgType> explicit exception_base(const Any& msg) :
				 _what{msg}
			{
			}

			// virtual const char* what() const noexcept override { return this->___what.c_str(); }
			const MsgType& what() const noexcept { return this->_what; }

			// virtual const str& what() const noexcept { return this->___what; }
			// virtual str_v what_v() const noexcept { return this->___what; }
		};

		/**
		 * @brief simplest exception
		 */
		template<typename MsgType>
		struct exception : public exception_base<exception<MsgType>, MsgType>
		{
			using exception_base<exception<MsgType>, MsgType>::exception_base;
		};

		/**
		 * @brief default exception, that is raised on failed check
		 */
		template<typename MsgType>
		struct assert_exception : public exception_base<assert_exception<MsgType>, MsgType>
		{
			using exception_base<assert_exception<MsgType>, MsgType>::exception_base;
		};

		/**
		 * @brief this exception should be thrown if something is not found
		 */
		template<typename MsgType>
		struct not_found_exception : public exception_base<not_found_exception<MsgType>, MsgType>
		{
			using exception_base<not_found_exception<MsgType>, MsgType>::exception_base;
		};

		/**
		 * @brief this exception should be thrown if given pointer is nullptr, but shouldn't
		 */
		template<typename MsgType>
		struct pointer_is_null : public exception_base<pointer_is_null<MsgType>, MsgType>
		{
			using exception_base<pointer_is_null<MsgType>, MsgType>::exception_base;
		};

		/**
		 * @brief same as exception, but additionally prints reason to stdout, usefull, if extended log is required
		 */
		template<typename MsgType>
		struct tee_exception : public exception_base<tee_exception<MsgType>, MsgType>
		{
			template<typename U>
			explicit tee_exception(const U& msg) : exception_base<tee_exception<MsgType>, MsgType>{msg}
			{
				this->get_logger().error(this->what());
			}
		};

		/**
		 * @brief verifies is given type has minimum requirements for project-wide exceptions
		 * 
		 * @tparam T type to check
		 */
		template<template<typename Msg> typename T> concept supported_exception = requires
		{
			std::is_base_of_v<std::exception, T<core::str>>;
		};

		/**
		 * @brief alternative to asertion
		 * 
		 * @tparam _ExceptionType exception to throw if check failed
		 */
		template<template<typename Msg> typename _ExceptionType = exception, bool __log_pass = false>
		requires supported_exception<_ExceptionType> struct require :
			 Log<require<_ExceptionType, __log_pass>>
		{
			using Log<require<_ExceptionType, __log_pass>>::get_logger;

			/**
			 * @brief Construct a new require object, which is also checker
			 * 
			 * @tparam MsgType deducted type, of message
			 * @tparam ExceptionArgs variadic type of additional argument that are passed to exception constructor
			 * @param _check value to be checked
			 * @param msg message to pass to exception
			 * @param argv optional exception arguments
			 */
			template<typename MsgType, typename... ExceptionArgs>
			explicit require(const bool _check, const MsgType& msg = "no message provided",
								  ExceptionArgs&&... argv)
			{
				if(_check) [[likely]]	// be optimist :)
				{
					if constexpr(__log_pass)
						get_logger() << "passed: `" << msg << "`\n";	  // << logger::endl;
				}
				else [[unlikely]]
				{
					get_logger().error() << "Failed on check: " << msg << logger::endl;
					get_logger().print_stacktrace();
					throw _ExceptionType<MsgType>(msg, std::forward<ExceptionArgs>(argv)...);
				}
			}
		};

		/**
		 * @brief struct for checking is pointer nullptr
		 */
		struct require_not_nullptr
		{
			/**
			 * @brief message to show, when pointer is nullptr
			 */
			constexpr static str_v c_require_not_nullptr{"given pointer is equal to nullptr!"};
			using spec_require = require<pointer_is_null>;

			/**
			 * @brief by constructing object, checks, is given pointer, not a nullptr
			 * 
			 * @tparam T any type
			 * @param ptr pointer to validate
			 */
			template<typename T> require_not_nullptr(T* ptr) { check_impl(ptr); }
			template<typename T> require_not_nullptr(const T* ptr) { check_impl(ptr); }

			template<typename T, typename Deleter>
			require_not_nullptr(const std::unique_ptr<T, Deleter>& ptr)
			{
				check_impl(ptr.get());
			}
			template<typename T, typename Deleter>
			require_not_nullptr(const std::unique_ptr<T[], Deleter>& ptr)
			{
				check_impl(ptr.get());
			}

			template<typename T> require_not_nullptr(const std::shared_ptr<T>& ptr)
			{
				check_impl(ptr.get());
			}
			template<typename T> require_not_nullptr(const std::shared_ptr<T[]>& ptr)
			{
				check_impl(ptr.get());
			}

			template<typename T> require_not_nullptr(const std::weak_ptr<T>& ptr)
			{
				check_impl(ptr.get());
			}
			template<typename T> require_not_nullptr(const std::weak_ptr<T[]>& ptr)
			{
				check_impl(ptr.get());
			}

		 private:
			template<typename T> void check_impl(T* ptr) const
			{
				spec_require{ptr != nullptr, c_require_not_nullptr};
			}
			template<typename T> void check_impl(const T* ptr) const
			{
				spec_require{ptr != nullptr, c_require_not_nullptr};
			}
		};

		/**
		 * @brief object representation of error summary
		 */
		struct error_report
		{
			str reason;
			str callstack;

			error_report() = default;
			/**
			 * @brief Construct a new error report object
			 * 
			 * @param msg reason of exception
			 */
			explicit error_report(const str& msg) : reason{ msg } { this->set_callstack(); }

			/**
			 * @brief Construct a new error report object
			 * 
			 * @param ex any exception
			 */
			explicit error_report( const exception<str>& ex ) : error_report{ ex.what() } {}

			/**
			 * @brief Construct a new error report object
			 * 
			 * @param ex any exception
			 */
			explicit error_report( const exception<u16str>& ex ) : error_report{ core::get_conversion_engine().to_bytes( ex.what() ) } {}

			/**
			 * @brief returns the callstack as string object
			 * 
			 * @return str callstack
			 */
			static str get_callstack_as_string() 
			{
				std::stringstream ss;
				ss << boost::stacktrace::stacktrace();
				return ss.str();
			}

		private:

			/**
			 * @brief Set the callstack object
			 */
			void set_callstack()
			{
				callstack = get_callstack_as_string();
			}
		};
	};	  // namespace exceptions

	/**
	 * @brief alias for assertion project-wide [ c(ustom) assert ]
	 * 
	 * @tparam Ex_t exception to throw, by default assert_exception
	 */
	template<template<typename MsgT> typename Ex_t = exceptions::assert_exception>
	using cassert = typename exceptions::require<Ex_t>;

	/** [ d(efault) asssert ] */
	using dassert = cassert<>;

	/** [ v(erbouse) d(efault) asssert ] */
	using vdassert = exceptions::require<exceptions::assert_exception, true>;

	/** checks, whether given pointer is nullptr abnd throws exception if yes */
	using check_nullptr = exceptions::require_not_nullptr;

	/** @brief this namespace contains string utilities */
	namespace string_utils
	{
		/**
		 * @brief contains implementations, that shouldn't be used from outside
		 */
		namespace detail
		{
			/**
			 * @brief base class for splitters
			 * 
			 * @tparam string_view_type any stirng view type ex.: str_v or u16str_v
			 */
			template<typename string_view_type> struct split_words_base
			{
				using svt = string_view_type;
				using cht = svt::value_type;
				svt view;
				cht separator;
			};

			/**
			 * @brief base class for splitter iterators
			 * 
			 * @tparam string_view_type any stirng view type ex.: str_v or u16str_v
			 */
			template<typename string_view_type> struct iterator_base
			{
				using svt = typename split_words_base<string_view_type>::svt;
				using cht = typename split_words_base<string_view_type>::cht;

				/**
				 * @brief Construct a new iterator base object
				 * 
				 * @param v view, that is currently iterated
				 * @param start_here position to start searching
				 * @param sep separator of words
				 */
				iterator_base(svt v, const size_t start_here, const cht sep) :
					 view{v}, pos{start_here}, separator{sep}
				{
				}

				iterator_base& operator=(const iterator_base&) = default;
				iterator_base& operator=(iterator_base&&) = default;
				iterator_base(iterator_base&&)				= default;
				iterator_base(const iterator_base&)			= default;

				void operator++() { move(); }
				void operator++(int) { move(); }

				svt operator*() const { return get(); }
				svt operator->() const { return get(); }

				virtual ~iterator_base() {}

				inline friend bool operator==(const iterator_base& it1, const iterator_base& it2)
				{
					return (it1.pos == it2.pos);
				}
				inline friend bool operator!=(const iterator_base& it1, const iterator_base& it2)
				{
					return !(it1 == it2);
				}

			 protected:
				svt view;
				size_t pos;
				cht separator;
				size_t next_pos{svt::npos};

				/** @brief this method is called in operator++ */
				virtual void move() = 0;

			 private:
				/**
				 * @brief returns view on range between pos and next_pos
				 * 
				 * @return svt specified string view type
				 */
				svt get() const
				{
					const size_t next = (next_pos == svt::npos ? view.size() : next_pos);
					return svt{view.data() + pos + (pos != 0ul), next - pos - (pos != 0ul)};
				}
			};
		}	 // namespace detail

		using detail::iterator_base;
		using detail::split_words_base;

		/**
		 * @brief forward wrapper for splitting view
		 * 
		 * @tparam string_view_type any string view type
		 * @example split_words ~ usage
		 * ```
		 * constexpr str_v words{ "word1 word2 word3" };
		 * for(str_v word : split_words<str_v>{words, ' '}) log << word << logger::endl;
		 * ```
		 * 
		 * prints: 
		 * word1
		 * word2
		 * word3
		 */
		template<typename string_view_type>
		struct split_words : public split_words_base<string_view_type>
		{
			using svt = split_words_base<string_view_type>::svt;

			class iterator : public iterator_base<string_view_type>
			{
				using it_base = iterator_base<string_view_type>;
				using svt	  = typename it_base::svt;

				/** @brief calculates next_pos, basing on pos */
				void get_next_pos()
				{
					this->next_pos = this->view.find_first_of(this->separator, this->pos + 1ul);
				}

			 protected:
				/** @brief defines how to move between words */
				virtual void move() override
				{
					dassert{this->pos < this->view.size(), "it's end"_u8};
					this->pos = this->next_pos;
					get_next_pos();
				}

			 public:
				/** @brief forward constructor */
				template<typename... U> iterator(U&&... u) : it_base{std::forward<U>(u)...}
				{
					if(this->pos != svt::npos) get_next_pos();
				}
			};

			/**
			 * @brief return iterator, that points to first word
			 * 
			 * @return iterator 
			 */
			iterator begin() const
			{
				return iterator{this->view,
									 (this->view.size() == 0 ? svt::npos : 0ul),
									 this->separator};
			}

			/**
			 * @brief returns iterator that indicates end of given string
			 * 
			 * @return iterator 
			 */
			iterator end() const { return iterator{this->view, svt::npos, this->separator}; }
		};

		/**
		 * @brief wrapper for reverted splittied view
		 * 
		 * @tparam string_view_type any string view type
		 * @example split_words_rev ~ usage
		 * ```
		 * constexpr str_v words{ "word1 word2 word3" };
		 * for(str_v word : split_words_rev<str_v>{words, ' '}) log << word << logger::endl;
		 * ```
		 * 
		 * prints: 
		 * word3
		 * word2
		 * word1
		 */
		template<typename string_view_type>
		struct split_words_rev : public split_words_base<string_view_type>
		{
			using svt = split_words_base<string_view_type>::svt;

			class iterator : public iterator_base<string_view_type>
			{
				using it_base = iterator_base<string_view_type>;
				using svt	  = typename it_base::svt;

				/** @brief calculates next_pos and pos */
				void set_prev_pos()
				{
					this->next_pos = this->pos;
					if(this->next_pos != svt::npos)
						this->pos = this->view.find_last_of(this->separator, this->pos - 1ul);
				}

			 protected:
				/** @brief defines how to move between words */
				virtual void move() override
				{
					dassert{this->next_pos != 0, "cannot decrement from beginning"_u8};
					set_prev_pos();
				}

			 public:
				/** @brief forward constructor */
				template<typename... U> iterator(U&&... u) : it_base{std::forward<U>(u)...}
				{
					if(this->pos != svt::npos) set_prev_pos();
				}
			};

			/**
			 * @brief return iterator, that points to last word
			 * 
			 * @return iterator 
			 */
			iterator begin() const
			{
				return iterator{this->view,
									 (this->view.size() == 0 ? svt::npos : this->view.size()),
									 this->separator};
			}

			/**
			 * @brief returns iterator that indicates running out of words
			 * 
			 * @return iterator 
			 */
			iterator end() const { return iterator{this->view, svt::npos, this->separator}; }
		};
	}	 // namespace string_utils

}	 // namespace core

/** @brief automatic conversion for logger */
template<>
typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str_v& v);

/** @brief automatic conversion for logger */
template<>
typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str& v);