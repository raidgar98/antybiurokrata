#include <antybiurokrata/libraries/demangler/demangler.h>
#include <ranges>

template <>
void core::demangler::mangle<core::demangler::conv_t::HTML>(core::str &out)
{
	if (out.size() == 0)
		return;

	auto converter = get_conversion_engine();
	core::str result;
	const std::string_view view{out};
	const std::ranges::split_view splitted{view, '&'};
	bool first_it = true;

	for (const auto &line : splitted)
	{
		const long length{ std::ranges::distance(line) };
		if(length == 0)
		{
			if(first_it) first_it = false;
			else result += '&';
			continue;
		}

		core::str tag{"&"}, rest; 
		tag.reserve(length);
		rest.reserve(length);

		core::str* save_point = &tag;

		// tag save to one varriable, rest of splitted data to another
		for (const auto c : line)
		{
			*save_point += c;
			if(c == ';') save_point = &rest;
		}

		// if first letter is not #, then it's not HTML tag
		if(tag[1] != '#') result += tag;
		else
		{
			const char16_t decoded = detail::depolonizator::reverse_get<core::demangler::conv_t::HTML>(tag);	// do lookup for specified tag
			if(decoded == '\0') result += tag;	// if not found add tag
			else result += converter.to_bytes(decoded);					// if found add decoded charachter
		}
		result += rest;
	}

	out = std::move(result);
}
