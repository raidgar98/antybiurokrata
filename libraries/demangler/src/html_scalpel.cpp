#include <antybiurokrata/libraries/html_scalpel/html_scalpel.h>

namespace core
{
	void html_scalpel(const str_v& input_html, std::vector<u16str>& output)
	{
		// first convert to u16
		constexpr size_t word_prereserve_size{20ul};
		auto converter = get_conversion_engine();
		const u16str wide_html{converter.from_bytes(input_html.data())};

		// implicit conversion to wchar_t is required, because std does not suppert u16 for isaplha and isdigit :(
		const auto is_correct = [](const wchar_t c) -> bool {
			constexpr std::wstring_view accepted{L"-_ \t&#+/.,"};
			return std::isalpha(c, plPL()) || std::isdigit(c, plPL())
					 || accepted.find(c) != std::wstring_view::npos;
		};

		// initialize required varriables
		u16str word;
		word.reserve(word_prereserve_size);
		bool in_tag				 = false;
		bool in_quote			 = false;
		bool ignore_next		 = false;
		bool is_double_spaced = false;

		for(const u16char_t c: wide_html)
		{
			if(ignore_next) ignore_next = false;
			else if(u'\\' == c)
				ignore_next = true;
			else if(in_tag && (u'"' == c || u'\'' == c))
				in_quote = !in_quote;
			else if(!in_tag && u'<' == c)
				in_tag = true;
			else if(in_tag && u'>' == c)
			{
				in_tag = false;
				if(!is_double_spaced)
				{
					// output += ' ';
					word.shrink_to_fit();
					output.emplace_back(std::move(word));
					word.clear();
					word.reserve(word_prereserve_size);
					is_double_spaced = true;
				}
			}
			else if(!in_tag && is_correct(c))
			{
				if(is_double_spaced && u'.' == c) continue;
				if(u' ' == c || u'\t' == c)
				{
					if(is_double_spaced) continue;
					else
						is_double_spaced = true;
				}
				else
					is_double_spaced = false;

				if(u',' == c) is_double_spaced = true;

				word += c;
			}
		}
	}
}	 // namespace core