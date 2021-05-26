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
#include <regex>
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
			ENG  = 0, /** @brief this will just replace polish 'ąśćźżółę' to 'asczzole' */
			HTML = 1, /** @brief this will replace polish letters to html codes (sanitizing) */
			URL  = 2 /**  @brief this will replace polish letters to unicode in UTF-8 transformation */
		};

		/** @brief type used in internal map in letter_converter as key */
		using translation_key_t = u16char_t;

		/** @brief type used in internal map in letter_converter as value */
		struct translation_value_t
		{
			const u16char_t eng;
			const u16str html;
			const u16str url;

			translation_value_t(const u16char_t c, const u16str& s, const u16str& u) :
				 eng{c}, html{s}, url{u} {};

			/** @brief optimalized accessor */
			template<conversion_t type> auto get() const
			{
				if constexpr(conversion_t::ENG == type) return eng;
				else if constexpr(conversion_t::HTML == type)
					return u16str_v{html};
				else if constexpr(conversion_t::URL == type)
					return u16str_v{url};

				core::dassert{false, "invalid decision path"_u8};
				return decltype(get<type>()){};
			}
		};

		/**
		 * @brief provides polish -> international translation
		*/
		struct depolonizator : Log<depolonizator>
		{
			using translation_map_t = std::map<translation_key_t, translation_value_t>;
			using kv_type				= std::pair<const translation_key_t, translation_value_t>;
			using iterator_t			= decltype(((translation_map_t*)(nullptr))->cbegin());

			/** @brief source of encies http://taat.pl/en/narzedzia/utf-8/#tab18 */
			inline static const translation_map_t translation{std::initializer_list<kv_type>{

				 // polish chars
				 kv_type{u'ą', translation_value_t{u'a', u"&#0261", u"%C4%85"}},
				 {u'ć', {u'c', u"&#0263", u"%C4%87"}},
				 {u'ę', {u'e', u"&#0281", u"%C4%99"}},
				 {u'ł', {u'l', u"&#0322", u"%C5%82"}},
				 {u'ń', {u'n', u"&#0324", u"%C5%84"}},
				 {u'ś', {u's', u"&#0347", u"%C5%9B"}},
				 {u'ó', {u'o', u"&#0243", u"%C3%B3"}},
				 {u'ź', {u'z', u"&#0378", u"%C5%BC"}},
				 {u'ż', {u'z', u"&#0380", u"%C5%BA"}},
				 {u'Ą', {u'A', u"&#0260", u"%C4%84"}},
				 {u'Ć', {u'C', u"&#0262", u"%C4%86"}},
				 {u'Ę', {u'E', u"&#0280", u"%C4%98"}},
				 {u'Ł', {u'L', u"&#0321", u"%C5%81"}},
				 {u'Ń', {u'N', u"&#0323", u"%C5%83"}},
				 {u'Ś', {u'S', u"&#0346", u"%C5%9A"}},
				 {u'Ó', {u'O', u"&#0211", u"%C3%93"}},
				 {u'Ź', {u'Z', u"&#0377", u"%C5%BB"}},
				 {u'Ż', {u'Z', u"&#0379", u"%C5%B9"}},

				 // special chars
				 {u'_', {u'_', u"&#0095", u"_"}},
				 {u' ', {u' ', u"&#0032", u"+"}},
				 {u'-', {u'-', u"&#0045", u"-"}}}};

			static std::tuple<bool, iterator_t> safe_get(translation_key_t letter)
			{
				const iterator_t found = translation.find(letter);
				return {found != translation.end(), found};
			}

			/** @brief optimalized accessor */
			template<conversion_t type> static str_v get(translation_key_t letter)
			{
				return translation.at(letter).get<type>();
			}

			template<conversion_t type> static char16_t reverse_get(const u16str_v& tag)
			{
				for(const auto& kv: translation)
					if(kv.second.get<type>() == tag) return kv.first;
				get_logger().warn() << "tag `" << tag << "` not found" << logger::endl;
				return u'\0';
			}
		};
	}	 // namespace detail
	using conv_t = detail::conversion_t;

	/** @brief this narrows demangler to types used in project, no need to add support for more string types */
	template<typename string_type>
	concept acceptable_string_types_req
		 = std::is_same_v<string_type, str> || std::is_same_v<string_type, u16str>;

	/** @brief this narrows demangler to types used in project, no need to add support for more string view types */
	template<typename string_view_type>
	concept acceptable_string_view_types_req
		 = std::is_same_v<string_view_type, str_v> || std::is_same_v<string_view_type, u16str_v>;

	template<acceptable_string_types_req string_type			  = str,
				acceptable_string_view_types_req string_view_type = str_v>
	class demangler : public Log<demangler<string_type, string_view_type>>
	{
		using Log<demangler>::log;

		string_type data;
		bool processed{false};

	 public:
		using modify_fun_t									= std::function<char_t(char_t)>;
		inline static const modify_fun_t default_fun = [](char_t c) { return c; };

		template<typename... U> explicit demangler(U&&... u) : data{std::forward<U>(u)...} {}

		/**
		 * @brief performs processing, and guards to not do it twice
		 * 
		 * @tparam type format of replacement
		 * @return demangler& self
		 */
		template<conv_t type> demangler& process()
		{
			if(!processed)
			{
				demangler<>::demangle<type>(data);
				processed = true;
			}
			return *this;
		}

		/**
		 * @brief proxy to process<>
		 * 
		 * @return demangler& self
		 */
		demangler& operator()() { return process<conv_t::ENG>(); }

		/**
		 * @brief gets view on processed data
		 * 
		 * @throws assert_exception if data is not processed yet
		 * @return string_view_type 
		 */
		string_view_type get() const
		{
			dassert(processed, "data is not processed yet, run `process` first"_u8);
			return string_view_type{data};
		}

		/**
		 * @brief gets copy of processed data
		 * 
		 * @throws assert_exception if data is not processed yet
		 * @return string_type
		 */
		string_type get_copy() const
		{
			dassert(processed, "data is not processed yet, run `process` first"_u8);
			return data;
		}

		/**
		 * @brief proxy to mangle(u16str&)
		 * 
		 * @tparam type format of replacement
		 * @param out input and output
		 */
		template<conv_t type> static void mangle(str& out)
		{
			auto conv = core::get_conversion_engine();
			u16str tmp{conv.from_bytes(out)};
			mangle<type>(tmp);
			out = std::move(conv.to_bytes(tmp));
		}

		/**
		 * @brief do oposite job to demangle, works only for HTML
		 * 
		 * @tparam type format of replacement
		 * @param out input and output
		 */
		template<conv_t type>
		requires(type == conv_t::HTML || type == conv_t::URL) static void mangle(u16str& out)
		{
			if constexpr(type == conv_t::HTML) demangler<>::mangle_html(out);
			else if constexpr(type == conv_t::URL)
				demangler<>::mangle_url(out);
			else
				core::dassert(false, "invalid decision path"_u8);
		}

		/**
		 * @brief implementation of mangle for HTML convertion tag
		 * 
		 * @param out input nad output
		 */
		static void mangle_html(u16str& out);

		/**
		 * @brief implementation of mangle for URL convertion type
		 * 
		 * @param out input and output
		 */
		static void mangle_url(u16str& out);

		/**
		 * @brief trivially way of gussing is string is in polish lang? It demangles to english, and returns comprasion between them
		 * 
		 * @param view string to check is polish
		 * @return true if given string is in polish language
		 * @return false otherwise
		 */
		static bool is_polish(u16str_v view)
		{
			demangler<u16str, u16str_v> dmg{view};
			dmg.process<conv_t::ENG>();
			return dmg.get() == view;
		}

		/**
		 * @brief unifies given string to comparable format accross whole project
		 * 
		 * @param out input and output
		 */
		static void sanitize(u16str& out)
		{
			if(out.empty()) return;
			u16str ret;
			ret.reserve(out.size());

			demangler<u16str, u16str_v> dmg{out};
			u16str_v preprocessed = dmg.process<conv_t::ENG>().get();

			const std::locale loc{plPL()};
			for(const u16char_t c: preprocessed)
				if(std::isalnum<wchar_t>(
						 static_cast<wchar_t>(c),
						 loc) /* || u' ' == c */)	 // possiblility of problem with diffrence in double space
					ret += static_cast<u16char_t>(std::toupper<wchar_t>(static_cast<wchar_t>(c), loc));

			out = std::move(ret);
		}

		/**
		 * @brief do all job, by replacing polish chars to selected one
		 * 
		 * @tparam type format of replacement
		 * @param out input and output as u16str
		 */
		template<conv_t type> static void demangle(u16str& out)
		{
			if(out.size() == 0) return;

			u16str wout{};

			if constexpr(conv_t::HTML == type) wout.reserve(4 * out.size());
			else
				wout.reserve(out.size());

			// for(const u16char_t c: out)
			for(auto it = out.begin(); it != out.end(); ++it)
			{
				const u16char_t c = *it;
				if(
					 // if it's ENG, just check if it's in ASCII range, otherwise exclude ' _-' chars
					 c <= std::numeric_limits<char_t>::max()
					 && (conv_t::ENG == type or (u'_' != c and u' ' != c and u'-' != c)))
					 [[likely]]	  // in polish language most of letters are in <0;255> ASCII range
				{
					wout += c;
				}
				else
				{
					const auto [found, it] = detail::depolonizator::safe_get(c);
					if(found) wout += it->second.get<type>();
					else
						wout += c;
				}
			}

			wout.shrink_to_fit();
			out = std::move(wout);
		}

		/**
		 * @brief proxy to demangle(u16str&)
		 * 
		 * @tparam type format of replacement
		 * @param out input and output as str
		 */
		template<conv_t type> static void demangle(str& out)
		{
			auto conv = core::get_conversion_engine();
			u16str tmp{conv.from_bytes(out)};
			demangle<type>(tmp);
			out = std::move(conv.to_bytes(tmp));
		}
	};
}	 // namespace core

// template <>
// void core::demangler<>::mangle<core::demangler<>::conv_t::HTML>(core::str &out);