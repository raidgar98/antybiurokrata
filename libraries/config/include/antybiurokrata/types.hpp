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
	struct u16str_pretty_serial;

	/** @brief handy type for wide type */
	template<auto X> using u16ser = patterns::serial::ser<X, u16str, u16str_serial, u16str_deserial, u16str_pretty_serial>;

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
		struct deletable_codecvt : public codecvt_t
		{
			~deletable_codecvt() {}
		};
		return std::wstring_convert<deletable_codecvt, u16char_t>{"Error", u"Error"};
	}

	struct u16str_serial
	{
		template<typename stream_type> u16str_serial(stream_type& os, const u16str_v& view)
		{
			using patterns::serial::delimiter;
			os << view.size() << delimiter;
			for(const auto c: view) os << static_cast<int>(c) << delimiter;
		}
	};

	struct u16str_deserial
	{
		template<typename stream_type> u16str_deserial(stream_type& is, u16str& out)
		{
			using patterns::serial::delimiter;
			size_t size;
			is >> size;
			if(size == 0) return;
			else
				out.reserve(size);
			for(size_t i = 0; i < size; ++i)
			{
				int c;
				is >> c;
				out += static_cast<u16char_t>(c);
			}
		}
	};

	struct u16str_pretty_serial
	{
		template<typename stream_type> u16str_pretty_serial(stream_type& os, const u16str_v& view)
		{
			using patterns::serial::delimiter;
			os << get_conversion_engine().to_bytes(view.data());
		}
	};

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
		template<typename T, typename MsgType> struct exception_base : /* public std::exception,*/ Log<exception_base<T, MsgType>>
		{
			using Log<exception_base<T, MsgType>>::get_logger;

			/** stores message */
			const MsgType _what;

			template<typename Any> requires std::is_convertible_v<Any, MsgType> explicit exception_base(const Any& msg) : _what{msg}
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
		template<typename MsgType> struct exception : public exception_base<exception<MsgType>, MsgType>
		{
		};

		/**
		 * @brief default exception, that is raised on failed check
		 */
		template<typename MsgType> struct assert_exception : public exception_base<assert_exception<MsgType>, MsgType>
		{
			using exception_base<assert_exception<MsgType>, MsgType>::exception_base;
		};

		/**
		 * @brief same as exception, but additionally prints reason to stdout, usefull, if extended log is required
		 */
		template<typename MsgType> struct tee_exception : public exception_base<tee_exception<MsgType>, MsgType>
		{
			template<typename U> explicit tee_exception(const U& msg) : exception_base<tee_exception<MsgType>, MsgType>{msg}
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
		requires supported_exception<_ExceptionType> struct require : Log<require<_ExceptionType, __log_pass>>
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
			explicit require(const bool _check, const MsgType& msg = "no message provided", ExceptionArgs&&... argv)
			{
				if(_check) [[likely]]	// be optimist :)
				{
					if constexpr(__log_pass) get_logger() << "passed: `" << msg << "`\n";	// << logger::endl;
				}
				else [[unlikely]]
				{
					get_logger().error() << "Failed on check: " << msg << logger::endl;
					get_logger().print_stacktrace();
					throw _ExceptionType<MsgType>(msg, std::forward<ExceptionArgs>(argv)...);
				}
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

	/** @brief this namespace contains string utilities */
	namespace string_utils
	{
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
				iterator_base(svt v, const size_t start_here, const cht sep) : view{v}, pos{start_here}, separator{sep} {}

				iterator_base& operator=(const iterator_base&) = default;
				iterator_base& operator=(iterator_base&&) = default;
				iterator_base(iterator_base&&)				= default;
				iterator_base(const iterator_base&)			= default;

				void operator++() { move(); }
				void operator++(int) { move(); }

				svt operator*() const { return get(); }
				svt operator->() const { return get(); }

				virtual ~iterator_base() {}

				inline friend bool operator==(const iterator_base& it1, const iterator_base& it2) { return (it1.pos == it2.pos); }
				inline friend bool operator!=(const iterator_base& it1, const iterator_base& it2) { return !(it1 == it2); }

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
		template<typename string_view_type> struct split_words : public split_words_base<string_view_type>
		{
			using svt = split_words_base<string_view_type>::svt;

			class iterator : public iterator_base<string_view_type>
			{
				using it_base = iterator_base<string_view_type>;
				using svt	  = typename it_base::svt;

				/** @brief calculates next_pos, basing on pos */
				void get_next_pos() { this->next_pos = this->view.find_first_of(this->separator, this->pos + 1ul); }

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

			iterator begin() const { return iterator{this->view, (this->view.size() == 0 ? svt::npos : 0ul), this->separator}; }
			iterator end() const { return iterator{this->view, svt::npos, this->separator}; }
		};

		/**
		 * @brief revert wrapper for splitting view
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
		template<typename string_view_type> struct split_words_rev : public split_words_base<string_view_type>
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
					if(this->next_pos != svt::npos) this->pos = this->view.find_last_of(this->separator, this->pos - 1ul);
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

			iterator begin() const
			{
				return iterator{this->view, (this->view.size() == 0 ? svt::npos : this->view.size()), this->separator};
			}
			iterator end() const { return iterator{this->view, svt::npos, this->separator}; }
		};
	}	 // namespace string_utils

}	 // namespace core

template<> typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str_v& v);
template<> typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str& v);