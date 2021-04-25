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

template <>
typename logger::logger_piper &&operator<<<>(logger::logger_piper &&src, const core::u16str_v &v);
template <>
typename logger::logger_piper &&operator<<<>(logger::logger_piper &&src, const core::u16str &v);

namespace core
{
	namespace detail
	{
		/**
		 * @brief defines types of conversions
		 */
		enum conversion_t
		{
			ENG = 0,  /** @brief this will just replace polish 'ąśćźżółę' to 'asczzole' */
			HTML = 1, /** @brief this will replace polish letters to html codes (sanitizing) */
			URL = 2	  /**  @brief this will replace polish letters to unicode in UTF-8 transformation */
		};

		/** @brief type used in internal map in letter_converter as key */
		using translation_key_t = u16char_t;

		/** @brief type used in internal map in letter_converter as value */
		struct translation_value_t
		{
			const u16char_t eng;
			const u16str html;
			const u16str url;

			translation_value_t(const u16char_t c, const u16str &s, const u16str &u) : eng{c}, html{s}, url{u} {};

			/** @brief optimalized accessor */
			template <conversion_t type>
			auto get() const
			{
				if constexpr (conversion_t::ENG == type)
					return eng;
				else if constexpr (conversion_t::HTML == type)
					return u16str_v{html};
				else if constexpr (conversion_t::URL == type)
					return u16str_v{url};

				core::dassert{false, "invalid decision path"};
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
				kv_type{u'ą', translation_value_t{u'a', u"&#261;", u"#C4#85"}},
				{u'ć', {u'c', u"&#263;", u"#C4#87"}},
				{u'ę', {u'e', u"&#281;", u"#C4#99"}},
				{u'ł', {u'l', u"&#322;", u"#C5#82"}},
				{u'ń', {u'n', u"&#324;", u"#C5#84"}},
				{u'ś', {u's', u"&#347;", u"#C5#9B"}},
				{u'ó', {u'o', u"&#243;", u"#C3#B3"}},
				{u'ź', {u'z', u"&#378;", u"#C5#BC"}},
				{u'ż', {u'z', u"&#380;", u"#C5#BA"}},
				{u'Ą', {u'A', u"&#260;", u"#C4#84"}},
				{u'Ć', {u'C', u"&#262;", u"#C4#86"}},
				{u'Ę', {u'E', u"&#280;", u"#C4#98"}},
				{u'Ł', {u'L', u"&#321;", u"#C5#81"}},
				{u'Ń', {u'N', u"&#323;", u"#C5#83"}},
				{u'Ś', {u'S', u"&#346;", u"#C5#9A"}},
				{u'Ó', {u'O', u"&#211;", u"#C3#93"}},
				{u'Ź', {u'Z', u"&#377;", u"#C5#BB"}},
				{u'Ż', {u'Z', u"&#379;", u"#C5#B9"}},

				// special chars
				{u'_', {u'_', u"&#95;", u"_"}},
				{u' ', {u' ', u"&#32;", u"+"}},
				{u'-', {u'-', u"&#45;", u"-"}}}};

			static std::tuple<bool, iterator_t> safe_get(translation_key_t letter)
			{
				const iterator_t found = translation.find(letter);
				return {found != translation.end(), found};
			}

			/** @brief optimalized accessor */
			template <conversion_t type>
			static str_v get(translation_key_t letter) { return translation.at(letter).get<type>(); }

			template <conversion_t type>
			static char16_t reverse_get(const u16str_v &tag)
			{
				for (const auto &kv : translation)
					if (kv.second.get<type>() == tag)
						return kv.first;
				get_logger().warn() << "tag `" << tag << "` not found" << logger::endl;
				return u'\0';
			}
		};
	}
	using conv_t = detail::conversion_t;

	/** @brief this narrows demangler to types used in project, no need to add support for more string types */
	template <typename string_type>
	concept acceptable_string_types_req = std::is_same_v<string_type, str> || std::is_same_v<string_type, u16str>;

	/** @brief this narrows demangler to types used in project, no need to add support for more string view types */
	template <typename string_view_type>
	concept acceptable_string_view_types_req = std::is_same_v<string_view_type, str_v> || std::is_same_v<string_view_type, u16str_v>;

	template <acceptable_string_types_req string_type = str, acceptable_string_view_types_req string_view_type = str_v>
	class demangler : public Log<demangler<string_type, string_view_type>>
	{
		using Log<demangler>::log;

		string_type data;
		bool processed{false};

	public:
		using modify_fun_t = std::function<char_t(char_t)>;
		inline static const modify_fun_t default_fun = [](char_t c) { return c; };

		template <typename... U>
		explicit demangler(U &&...u) : data{std::forward<U>(u)...} {}

		/**
		 * @brief performs processing, and guards to not do it twice
		 * 
		 * @tparam type format of replacement
		 * @return demangler& self
		 */
		template <conv_t type>
		demangler &process()
		{
			if (!processed)
			{
				demangle<type>(data);
				processed = true;
			}
			return *this;
		}

		/**
		 * @brief proxy to process<>
		 * 
		 * @return demangler& self
		 */
		demangler &operator()()
		{
			return process<conv_t::ENG>();
		}

		/**
		 * @brief gets view on processed data
		 * 
		 * @throws assert_exception if data is not processed yet
		 * @return string_view_type 
		 */
		string_view_type get() const
		{
			dassert(processed, "data is not processed yet, run `process` first");
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
			dassert(processed, "data is not processed yet, run `process` first");
			return data;
		}

		/**
		 * @brief returns the conversion engine object
		 * 
		 * @return std::wstring_convert
		 */
		static auto get_conversion_engine()
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
		 * @brief proxy to mangle(u16str&)
		 * 
		 * @tparam type format of replacement
		 * @param out input and output
		 */
		template <conv_t type>
		static void mangle(str &out)
		{
			auto conv = get_conversion_engine();
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
		template <conv_t type>
		requires(type == conv_t::HTML || type == conv_t::URL) 
		static void mangle(u16str &out)
		{
			if constexpr (type == conv_t::HTML) mangle_html(out);
			else if constexpr (type == conv_t::URL) mangle_url(out);
			else core::dassert(false, "invalid decision path");
		}

private:

		/**
		 * @brief implementation of mangle for HTML convertion tag
		 * 
		 * @param out input nad output
		 */
		static void mangle_html(u16str &out)
		{
			static const auto valid_html_tag = [](const u16str_v &view) -> bool {
				if (view.size() < 4)
					return false;
				if (view.at(0) != u'&')
					return false;
				if (view.at(1) != u'#')
					return false;
				if (view.at(view.size() - 1) != u';')
					return false;
				for (size_t i = 2; i < view.size() - 1; ++i)
					if (!std::iswdigit(view.at(i)))
						return false;
				return true;
			};

			if (out.size() == 0)
				return;

			u16str_v view{out};
			const std::ranges::split_view splitted{view, u'&'};
			u16str result;
			bool first_it = true;

			for (auto line_it = splitted.begin(); line_it != splitted.end(); line_it++)
			{
				const auto& line = *line_it;
				const long length{std::ranges::distance(line)};
				if (length == 0)
				{
					if (first_it) first_it = false;
					else result += u'&';
					continue;
				}

				u16str tag{u"&"};
				if(line_it == splitted.begin()) tag.clear();
				tag.reserve(length);
				u16str rest;
				rest.reserve(length);

				u16str *save_point = &tag;

				// tag save to one varriable, rest of splitted data to another
				for (const u16char_t c : line)
				{
					*save_point += c;
					if (c == u';')
						save_point = &rest;
				}

				// if given text does not match regex, just add this as whole
				if (!valid_html_tag(tag))
					result += tag;
				else
				{
					const u16char_t decoded = detail::depolonizator::reverse_get<conv_t::HTML>(tag); // do lookup for specified tag
					if (decoded == u'\0')
						result += tag; // if not found add tag
					else
						result += decoded; // if found add decoded charachter
				}
				result += rest;
			}

			result.shrink_to_fit();
			out = std::move(result);
		}

		/**
		 * @brief implementation of mangle for URL convertion type
		 * 
		 * @param out input and output
		 */
		static void mangle_url(u16str & out)
		{
			static const auto valid_url_tag = [](const u16str_v &view) -> bool {
				constexpr u16str_v hex_charachters{ u"0123456789ABCDEF" };
				if (view.size() != 6) return false;
				if (view.at(0) != u'#') return false;
				if (view.at(3) != u'#') return false;
				for(size_t i = 1; i < view.size(); ++i) if(i == 3) continue; else 
					if(hex_charachters.find(view[i]) == u16str::npos) return false;
				return true;
			};

			if (out.size() == 0) return;

			constexpr u16char_t hash{ u'#' };
			u16str_v view{out};
			const std::ranges::split_view splitted{view, hash};
			u16str result;
			bool first_it = true;
			bool in_middle = false;
			u16str current_tag{}; 
			current_tag.reserve(6);

			// for (const auto &line : splitted)
			for (auto line_it = splitted.begin(); line_it != splitted.end(); line_it++)
			{
				const auto& line = *line_it;

				const long length{std::ranges::distance(line)};
				if (length == 0)
				{
					if (first_it) first_it = false;
					else
					{
						if(!current_tag.empty()) // Ex. aaa#3F##D2aaa
						{
							result += current_tag;
							current_tag.clear(); 
							current_tag.reserve(6);
						}
						result += hash;
					}
					continue;
				}

				u16str tag{hash};
				if(line_it == splitted.begin()) tag.clear();
				tag.reserve(length);
				u16str rest;
				rest.reserve(length);

				u16str *save_point = &tag;

				// tag save to one varriable, rest of splitted data to another
				for (const u16char_t c : line)
				{
					*save_point += c;
					if (tag.size() == 3) save_point = &rest;
				}

				// here is decision block of collecting encies
				if(!current_tag.empty() && !tag.empty() && tag.starts_with(hash)) current_tag += tag; // Ex.: "aa#3F#D2aaa", here comes for "#D2" part
				else if(current_tag.empty() && !tag.empty() && rest.empty() && tag.starts_with(hash)) // Ex.: "aa#3F#D2aaa", here comes for "#3F" part
				{
					current_tag = tag;
					continue;
				}else if(current_tag.empty() && !tag.empty() && !rest.empty()) // Ex.: "aa#3Faaaa" and "3Faaaa"
				{
					result += tag + rest;
					continue;
				}else if(current_tag.empty() && !tag.starts_with(hash)) current_tag = tag;

				// if given text does not match regex, just add this as whole
				if (!valid_url_tag(current_tag)) result += current_tag;
				else
				{
					const u16char_t decoded = detail::depolonizator::reverse_get<conv_t::URL>(current_tag); // do lookup for specified tag
					if (decoded == u'\0') result += tag; // if not found add tag :(
					else result += decoded; // if found add decoded charachter
				}

				result += rest;
				current_tag.clear();
				current_tag.reserve(6);
			}
			result += current_tag;
			result.shrink_to_fit();
			out = std::move(result);
		}

		/**
		 * @brief do all job, by replacing polish chars to selected one
		 * 
		 * @tparam type format of replacement
		 * @param out input and output as u16str
		 */
		template <conv_t type>
		static void demangle(u16str &out)
		{
			if (out.size() == 0)
				return;

			u16str wout{};

			if constexpr (conv_t::HTML == type)
				wout.reserve(4 * wout.size());
			else
				wout.reserve(wout.size());

			for (const u16char_t c : out)
			{
				if (
					// if it's ENG, just check if it's in ASCII range, otherwise exclude ' _-' chars
					c <= std::numeric_limits<char_t>::max() and
					(conv_t::ENG == type or
					 (u'_' != c and u' ' != c and u'-' != c))) [[likely]] // in polish language most of letters are in <0;255> ASCII range
				{
					wout += c;
				}
				else
				{
					const auto [found, it] = detail::depolonizator::safe_get(c);
					if (found)
						wout += it->second.get<type>();
					else
						out += c;
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
		template <conv_t type>
		static void demangle(str &out)
		{
			auto conv = get_conversion_engine();
			u16str tmp{conv.from_bytes(out)};
			demangle<type>(tmp);
			out = std::move(conv.to_bytes(tmp));
		}
	};
}

// template <>
// void core::demangler<>::mangle<core::demangler<>::conv_t::HTML>(core::str &out);