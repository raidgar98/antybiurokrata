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
			for(auto x : identifier()) sum += x;
			return sum > 0;
		}

		str detail::detail_orcid_t::to_string(const detail::detail_orcid_t &orcid)
		{
			std::stringstream result;
			for (auto it = orcid.identifier().begin(); it != orcid.identifier().end(); it++)
			{
				if (it != orcid.identifier().begin())
					result << std::setw(1) << '-';
				result << std::setfill('0') << std::setw(4) << std::to_string(*it);
			}
			return result.str();
		}

		bool detail::detail_orcid_t::is_valid_orcid_string(const u16str_v &data, str* conversion_output)
		{
			const static std::regex orcid_validator_regex{"(\\d{4}-){3}\\d{4}"};
			const str conv{ get_conversion_engine().to_bytes(u16str(data)) };
			if(conversion_output) *conversion_output = conv;
			return std::regex_match(conv, orcid_validator_regex);
		}

		detail::detail_orcid_t detail::detail_orcid_t::from_string(const u16str_v &data)
		{
			str conv;
			dassert{is_valid_orcid_string(data, &conv), "given string is not valid ORCID number"};
			detail_orcid_t result{};

			size_t i = 0;
			for (str_v part : string_utils::split_words<str_v>{conv, '-'})
			{
				dassert(i < detail_orcid_t::words_in_orcid_num, "out of range");
				result.identifier()[i++] = static_cast<uint16_t>(std::stoi(part.data()));
			}

			return result;
		}

		void detail::detail_string_holder_t::set(const u16str_v &v)
		{
			using namespace core;
			
			if (v.size() == 0) return;
			const bool has_hashes{v.find(u'#') != u16str_v::npos};
			const bool has_ampersands{v.find(u'&') != u16str_v::npos};
			const bool has_percents{v.find(u'%') != u16str_v::npos};
			data(v);

			if (has_hashes && has_ampersands) demangler<>::mangle<conv_t::HTML>(data());
			else if(has_percents) demangler<>::mangle<conv_t::URL>(data());
		}

		bool detail::detail_polish_name_t::basic_validation(u16str_v input)
		{
			constexpr u16str_v allowed{u"-"}; // '-' for doubled surname
			if (input.size() <= 2) return false;
			bool bad_last = false;	// '-' cannot be at the end of surname
			for (const u16char_t c : input)
			{
				const bool is_letter{ std::isalpha(static_cast<wchar_t>(c), plPL()) };
				const bool is_allowed_char{ allowed.find(c) != u16str_v::npos };
				if(is_allowed_char) bad_last = true;
				else bad_last = false;

				if (!is_letter && !is_allowed_char) return false;
			}
			return !bad_last;
		}

		bool detail::detail_polish_name_t::is_valid() const
		{
			return basic_validation(data()().data());
		}

		void detail::detail_polish_name_t::unify() noexcept
		{
			for (u16char_t &c : data()().data())
				c = static_cast<u16char_t>(std::toupper(static_cast<wchar_t>(c), plPL()));
		}

		detail::detail_string_holder_t::operator str() const
		{
			return get_conversion_engine().to_bytes(data());
		}

		bool detail::detail_publication_t::compare(const detail::detail_publication_t & that) const
		{
			const detail::detail_publication_t& me = *this; // alias, to make it handy
			if(me.ids().size() > 0 && that.ids().size() > 0)
			{
				for(const auto& pair : me.ids())
				{
					const auto found = that.ids().find(pair.first);
					if(found != that.ids().end()) /* if found */ return pair.second() == found->second();
				}
			}

			// worst case
			return (me.year == that.year) && (me.title == that.title);
		}

	}
}