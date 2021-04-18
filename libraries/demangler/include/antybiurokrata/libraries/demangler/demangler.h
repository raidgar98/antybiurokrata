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
			ENG = 0, /** @brief this will just replace polish 'ąśćźżółę' to 'asczzole' */
			HTML = 1, /** @brief this will replace polish letters to html codes (sanitizing) */
			URL = 2 /**  @brief this will replace polish letters to unicode in UTF-8 transformation */
		};

		/** @brief type used in internal map in letter_converter as key */
		using translation_key_t = char16_t;

		/** @brief type used in internal map in letter_converter as value */
		struct translation_value_t
		{
			const char_t eng;
			const str html;
			const str url;

			translation_value_t(const char_t c, const str &s, const str& u) : eng{c}, html{s}, url{u} {};

			/** @brief optimalized accessor */
			template <conversion_t type>
			auto get() const
			{
				if 		constexpr (conversion_t::ENG == type ) return eng;
				else if constexpr ( conversion_t::HTML == type ) return html;
				else if constexpr ( conversion_t::URL == type ) return url;

				core::dassert{ false, "invalid decision path" };
				return decltype(get<type>()){};
			}
		};

		/**
		 * @brief provides polish -> international translation
		*/
		struct depolonizator : Log<depolonizator>
		{
			using translation_map_t = std::map<translation_key_t, translation_value_t>;
			using kv_type = std::pair<const translation_key_t, translation_value_t>;
			using iterator_t = decltype(((translation_map_t *)(nullptr))->cbegin());
			/** @brief source of encies http://taat.pl/en/narzedzia/utf-8/#tab18 */
			inline static const translation_map_t translation{std::initializer_list<kv_type>{
				
				 	// polish chars
			kv_type{u'ą', translation_value_t{'a', "&#261;", "#C4#85"}},
					{u'ć', {'c', "&#263;", "#C4#87"}},
					{u'ę', {'e', "&#281;", "#C4#99"}},
					{u'ł', {'l', "&#322;", "#C5#82"}},
					{u'ń', {'n', "&#324;", "#C5#84"}},
					{u'ś', {'s', "&#347;", "#C5#9B"}},
					{u'ó', {'o', "&#243;", "#C3#B3"}},
					{u'ź', {'z', "&#378;", "#C5#BC"}},
					{u'ż', {'z', "&#380;", "#C5#BA"}},
					{u'Ą', {'A', "&#260;", "#C4#84"}},
					{u'Ć', {'C', "&#262;", "#C4#86"}},
					{u'Ę', {'E', "&#280;", "#C4#98"}},
					{u'Ł', {'L', "&#321;", "#C5#81"}},
					{u'Ń', {'N', "&#323;", "#C5#83"}},
					{u'Ś', {'S', "&#346;", "#C5#9A"}},
					{u'Ó', {'O', "&#211;", "#C3#93"}},
					{u'Ź', {'Z', "&#377;", "#C5#BB"}},
					{u'Ż', {'Z', "&#379;", "#C5#B9"}},

					// special chars
					{u'_', {'_', "&#95;", "_"}},
					{u' ', {' ', "&#32;", "+"}},
					{u'-', {'-', "&#45;", "-"}}
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

			template<conversion_t type>
			static char16_t reverse_get(const str_v& tag)
			{
				for(const auto& kv : translation) if( kv.second.get<type>() == tag ) return kv.first;
				get_logger().warn() << "tag `" << tag << "` not found" << logger::endl;
				return u'\0';
			}
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

		/**
		 * @brief returns the conversion engine object
		 * 
		 * @return std::wstring_convert
		 */
		static auto get_conversion_engine()
		{
			return std::wstring_convert<

				deletable_facet<
					std::codecvt<
						char16_t, 
						char_t, 
						std::mbstate_t
					>
				>, 
				char16_t

			>{"Error", u"Error"};
		}

		/**
		 * @brief do oposite job to demangle, works only for HTML
		 * 
		 * @tparam type format of replacement
		 * @param out input and output
		 */
		template<conv_t type>
		static void mangle(str& out) {}

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

			auto conv = get_conversion_engine();
			std::u16string wout{conv.from_bytes(out)};
			std::string output{};
			if constexpr (conv_t::HTML == type) output.reserve(4 * wout.size());
			else output.reserve(wout.size());

			for (const auto c : wout)
			{
				if
				(
					// if it's ENG, just check if it's in ASCII range, otherwise exclude ' _-' chars
					c <= std::numeric_limits<char_t>::max() and 
					(
						conv_t::ENG == type or 
						( u'_' != c and u' ' != c and u'-' != c )
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

template<> void core::demangler::mangle<core::demangler::conv_t::HTML>(core::str &out);