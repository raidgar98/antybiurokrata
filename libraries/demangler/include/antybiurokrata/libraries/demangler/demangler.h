/**
 * @file demangler.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief in this file we can find objects responsible for sanitizing and converting polish strings
 * @version 0.1
 * @date 2021-04-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/types.hpp>
#include <codecvt>
#include <locale>
#include <string>
#include <tuple>
#include <map>

namespace core
{
	namespace detail
	{
		/**
		 * @brief defines types of conversions
		 */
		enum conversion_t
		{
			/** @brief this will just replace polish 'ąśćźżółę' to 'asczzole' */
			ENG = 0,

			/** @brief this will replace polish letters to html codes (sanitizing) */
			HTML = 1
		};

		/** @brief type used in internal map in letter_converter as key */
		using translation_key_t = wchar_t;

		/** @brief type used in internal map in letter_converter as value */
		struct translation_value_t
		{
			const str eng;
			const str html;

			translation_value_t(const char_t c, const str &s) : eng{1, c}, html{s} {};

			/** @brief optimalized accessor */
			template <conversion_t type>
			str_v get() const
			{
				if constexpr (conversion_t::ENG == type)
					return eng;
				else
					return html;
			}
		};

		/**
		 * @brief provides polish -> international translation
		*/
		struct depolonizator
		{
			using translation_map_t = std::map<translation_key_t, translation_value_t>;
			using kv_type = std::pair<const translation_key_t, translation_value_t>;
			using iterator_t = decltype(((translation_map_t *)(nullptr))->cbegin());
			/** @brief source of encies http://taat.pl/en/narzedzia/utf-8/#tab18 */
			inline const static translation_map_t translation{std::initializer_list<kv_type>{
				kv_type{L'ą', translation_value_t{'a', "&#261;"}},
				
				 	// polish chars
					{L'ć', {'c', "&#263;"}},
					{L'ę', {'e', "&#281;"}},
					{L'ł', {'l', "&#322;"}},
					{L'ń', {'n', "&#324;"}},
					{L'ś', {'s', "&#347;"}},
					{L'ó', {'o', "&#243;"}},
					{L'ź', {'z', "&#378;"}},
					{L'ż', {'z', "&#380;"}},
					{L'Ą', {'A', "&#260;"}},
					{L'Ć', {'C', "&#262;"}},
					{L'Ę', {'E', "&#280;"}},
					{L'Ł', {'L', "&#321;"}},
					{L'Ń', {'N', "&#323;"}},
					{L'Ś', {'S', "&#346;"}},
					{L'Ó', {'O', "&#211;"}},
					{L'Ź', {'Z', "&#377;"}},
					{L'Ż', {'Z', "&#379;"}},

					// special chars
					{L'_', {'_', "&#95;"}},
					{L' ', {' ', "&#32;"}},
					{L'-', {'-', "&#45;"}}
				}
			};

			static std::tuple<bool, iterator_t> safe_get(translation_key_t letter)
			{
				const iterator_t found = translation.find(letter);
				return {found != translation.end(), found};
			}

			/** @brief optimalized accessor */
			template <conversion_t type>
			static str_v get(translation_key_t letter) { return translation.at(letter).get<type>(); }
		};
	}

	class demangler : public Log<demangler>
	{
		using Log<demangler>::log;

		str data;
		bool processed{ false };

	public:

		using conv_t = detail::conversion_t;
		using modify_fun_t = std::function<char_t(char_t)>;
		inline static const modify_fun_t default_fun = [](char_t c) { return c; };

		template<typename ... U>
		explicit demangler(U&& ... u) : data{ std::forward<U>(u)... } {}

		/**
		 * @brief performs processing, and guards to not do it twice
		 * 
		 * @tparam type format of replacement
		 * @param fun additional processing of each letter
		 * @return demangler& self
		 */
		template<conv_t type>
		demangler& process(const modify_fun_t fun = default_fun) 
		{ 
			if(!processed)
			{
				demangle<type>(data, fun);
				processed = true;
			}
			return *this; 
		}

		/**
		 * @brief proxy to process<>
		 * 
		 * @param fun additional processing of each letter
		 * @return demangler& self
		 */
		demangler& operator()(const modify_fun_t fun = default_fun)
		{ 
			return process<conv_t::ENG>(fun); 
		}

		/**
		 * @brief gets view on processed data
		 * 
		 * @throws assert_exception if data is not processed yet
		 * @return str_v view of processed data
		 */
		str_v get() const 
		{
			dassert( processed, "data is not processed yet, run `process` first" );
			return str_v{data};
		}

	private:

		/** 
		 * @brief standard library abomination, to bypass deleter 
		 * @link https://en.cppreference.com/w/cpp/locale/codecvt#Example
		 */
		template<class Facet>
		struct deletable_facet : Facet
		{
			template<class ...Args>
			deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
			~deletable_facet() {}
		};

		/**
		 * @brief do all job, by replacing polish chars to selected one
		 * 
		 * @tparam type format of replacement
		 * @param out input and output
		 * @param fun additional processing of each letter 
		 */
		template <conv_t type>
		static void demangle(str &out, const modify_fun_t fun)
		{
			if (out.size() == 0) return;

			std::wstring_convert<deletable_facet<std::codecvt<wchar_t, char_t, std::mbstate_t>>> conv{"Error", L"Error"};
			std::wstring wout{conv.from_bytes(out)};
			std::string output{};
			if constexpr (conv_t::HTML == type) output.reserve(4 * wout.size());
			else output.reserve(wout.size());

			for (const wchar_t c : wout)
			{
				if
				(
					// if it's ENG, just check if it's in ASCII range, otherwise exclude ' _-' chars
					c <= std::numeric_limits<char_t>::max() and 
					(
						conv_t::ENG == type or 
						( L'_' != c and L' ' != c and L'-' != c )
					)
				) [[ likely ]] // in polish language most of letters are in <0;255> ASCII range
				{
					output += fun(static_cast<char_t>(c));
				}
				else
				{
					auto [found, it] = detail::depolonizator::safe_get(c);
					str tmp;

					if(found) tmp = it->second.get<type>();
					else tmp = conv.to_bytes(c);

					if constexpr ( conv_t::ENG == type )
					{
						dassert{ tmp.size() == 1, "return should contain only one char" };
						output += fun(tmp[0]);
					}else output += tmp;
				}
			}
			out = std::move(output);
		}
	};

}