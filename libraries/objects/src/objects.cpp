// STL
#include <ranges>
#include <regex>
#include <charconv>
#include <iomanip>

// Project
#include <antybiurokrata/libraries/objects/objects.h>
namespace core
{
	namespace objects
	{

		bool detail::detail_orcid_t::is_valid_orcid() const
		{
			size_t sum = 0;
			for(auto x: identifier()) sum += x;
			return sum > 0;
		}

		str detail::detail_orcid_t::to_string(const detail::detail_orcid_t& orcid)
		{
			std::stringstream result;
			for(auto it = orcid.identifier().begin(); it != orcid.identifier().end(); it++)
			{
				if(it != orcid.identifier().begin()) result << std::setw(1) << '-';
				result << std::setfill('0') << std::setw(4) << std::to_string(*it);
			}
			return result.str();
		}

		bool detail::detail_orcid_t::is_valid_orcid_string(const u16str_v& data,
																			str* conversion_output)
		{
			const str conv{get_conversion_engine().to_bytes(u16str(data))};
			if(conversion_output) *conversion_output = conv;
			return is_valid_orcid_string(conv);
		}


		bool detail::detail_orcid_t::is_valid_orcid_string(const str_v& data)
		{
			const static std::regex orcid_validator_regex{"([0-9]{4}-){3}[0-9]{4}"};
			if(data.empty() || data.size() > (words_in_orcid_num * 4) + (words_in_orcid_num - 1))
				return false;
			const str x{data};
			return std::regex_match(x, orcid_validator_regex);
		}

		detail::detail_orcid_t detail::detail_orcid_t::from_string(const u16str_v& data)
		{
			str conv;
			dassert{is_valid_orcid_string(data, &conv), "given string is not valid ORCID number"_u8};
			detail_orcid_t result{};

			size_t i = 0;
			for(str_v part: string_utils::split_words<str_v>{conv, '-'})
			{
				dassert(i < detail_orcid_t::words_in_orcid_num, "out of range"_u8);
				result.identifier()[i++] = static_cast<uint16_t>(std::stoi(part.data()));
			}

			return result;
		}

		detail::detail_orcid_t detail::detail_orcid_t::from_string(const str_v& data)
		{
			dassert{is_valid_orcid_string(data), "given string is not valid ORCID number"_u8};
			detail_orcid_t result{};

			size_t i = 0;
			for(str_v part: string_utils::split_words<str_v>{data, '-'})
			{
				dassert(i < detail_orcid_t::words_in_orcid_num, "out of range"_u8);
				result.identifier()[i++] = static_cast<uint16_t>(std::stoi(part.data()));
			}

			return result;
		}

		detail::polish_validator::operator bool() const noexcept
		{
			constexpr u16str_v allowed{u"-"};	// '-' for doubled surname
			if(x.size() <= 2) return false;
			bool bad_last = false;	 // '-' cannot be at the end of surname
			for(const u16char_t c: x)
			{
				const bool is_letter{std::isalpha(static_cast<wchar_t>(c), plPL())};
				const bool is_allowed_char{allowed.find(c) != u16str_v::npos};
				if(is_allowed_char) bad_last = true;
				else
					bad_last = false;

				if(!is_letter && !is_allowed_char) return false;
			}

			return !bad_last;
		}

		detail::polish_unifier::polish_unifier(u16str& x) noexcept
		{
			const auto ll = plPL();
			for(u16char_t& c: x) c = static_cast<u16char_t>(std::toupper(static_cast<wchar_t>(c), ll));
		}

		detail::ids_unifier::ids_unifier(u16str& x) noexcept
		{
			const auto ll = plPL();
			for(u16char_t& c: x)
			{
				const wchar_t wc = c;
				if(std::iswalnum(wc)) c = static_cast<u16char_t>(std::toupper(wc, ll));
			}
		}

		int detail::detail_publication_t::compare(const detail::detail_publication_t& that) const
		{
			static const auto calc = [](const auto& x1, const auto& x2) {
				if(x1 < x2) return -1;
				else if(x2 < x1)
					return 1;
				else
					return 0;
			};
			const detail::detail_publication_t& me = *this;	  // alias, to make it handy

			if(me.ids()()->size() > 0 && that.ids()()->size() > 0)
			{
				for(const auto& pair: me.ids()().data())
				{
					const auto found = that.ids()()->find(pair.first);
					if(found != that.ids()()->end()) /* if found */
						return calc(pair.second(), found->second());
				}
			}

			// worst case
			const int r_year = calc(me.year, that.year);
			if(r_year == 0)
			{
				const int r_title = calc(me.title()(), that.title()());
				if(r_title == 0 && !me.polish_title()()->empty() && !that.polish_title()()->empty())
					return calc(me.polish_title()(), that.polish_title()());
				else
					return r_title;
			}
			else
				return r_year;
		}

	}	 // namespace objects
}	 // namespace core