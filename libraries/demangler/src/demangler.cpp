#include <antybiurokrata/libraries/demangler/demangler.h>

template<> void core::demangler<>::mangle_html(core::u16str& out)
{
	static const auto valid_html_tag = [](const u16str_v& view) -> bool {
		if(view.size() < 6) return false;
		if(view.at(0) != u'&') return false;
		if(view.at(1) != u'#') return false;
		for(size_t i = 2; i < view.size(); ++i)
			if(!std::iswdigit(view.at(i))) return false;
		return true;
	};

	if(out.size() == 0) return;

	u16str_v view{out};
	const std::ranges::split_view splitted{view, u'&'};
	u16str result;
	bool first_it = true;

	for(auto line_it = splitted.begin(); line_it != splitted.end(); line_it++)
	{
		const auto& line = *line_it;
		const long length{std::ranges::distance(line)};
		if(length == 0)
		{
			if(first_it) first_it = false;
			else
				result += u'&';
			continue;
		}

		u16str tag{u"&"};
		if(line_it == splitted.begin()) tag.clear();
		tag.reserve(length);
		u16str rest;
		rest.reserve(length);

		u16str* save_point = &tag;

		// tag save to one varriable, rest of splitted data to another
		for(const u16char_t c: line)
		{
			*save_point += c;
			if(c == u';' || save_point->size() >= 6) save_point = &rest;
		}

		// if given text does not match regex, just add this as whole
		if(!valid_html_tag(tag)) result += tag;
		else
		{
			const u16char_t decoded = detail::depolonizator::reverse_get<conv_t::HTML>(tag);	  // do lookup for specified tag
			if(decoded == u'\0') result += tag;																  // if not found add tag
			else
				result += decoded;	// if found add decoded charachter
		}
		result += rest;
	}

	result.shrink_to_fit();
	out = std::move(result);
}

template<> void core::demangler<>::mangle_url(core::u16str& out)
{
	static const auto valid_url_tag = [](const u16str_v& view) -> bool {
		constexpr u16str_v hex_charachters{u"0123456789ABCDEF"};
		if(view.size() != 6) return false;
		if(view.at(0) != u'%') return false;
		if(view.at(3) != u'%') return false;
		for(size_t i = 1; i < view.size(); ++i)
			if(i == 3) continue;
			else if(hex_charachters.find(view[i]) == u16str::npos)
				return false;
		return true;
	};

	if(out.size() == 0) return;

	constexpr u16char_t hash{u'%'};
	u16str_v view{out};
	const std::ranges::split_view splitted{view, hash};
	u16str result;
	bool first_it	= true;
	bool in_middle = false;
	u16str current_tag{};
	current_tag.reserve(6);

	// for (const auto &line : splitted)
	for(auto line_it = splitted.begin(); line_it != splitted.end(); line_it++)
	{
		const auto& line = *line_it;

		const long length{std::ranges::distance(line)};
		if(length == 0)
		{
			if(first_it) first_it = false;
			else
			{
				if(!current_tag.empty())	// Ex. aaa#3F##D2aaa
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

		u16str* save_point = &tag;

		// tag save to one varriable, rest of splitted data to another
		for(const u16char_t c: line)
		{
			*save_point += c;
			if(tag.size() == 3) save_point = &rest;
		}

		// here is decision block of collecting encies
		if(!current_tag.empty() && !tag.empty() && tag.starts_with(hash))
			current_tag += tag;	 // Ex.: "aa#3F#D2aaa", here comes for "#D2" part
		else if(current_tag.empty() && !tag.empty() && rest.empty()
				  && tag.starts_with(hash))	// Ex.: "aa#3F#D2aaa", here comes for "#3F" part
		{
			current_tag = tag;
			continue;
		}
		else if(current_tag.empty() && !tag.empty() && !rest.empty())	 // Ex.: "aa#3Faaaa" and "3Faaaa"
		{
			result += tag + rest;
			continue;
		}
		else if(current_tag.empty() && !tag.starts_with(hash))
			current_tag = tag;

		// if given text does not match regex, just add this as whole
		if(!valid_url_tag(current_tag)) result += current_tag;
		else
		{
			const u16char_t decoded = detail::depolonizator::reverse_get<conv_t::URL>(current_tag);	// do lookup for specified tag
			if(decoded == u'\0') result += tag;																			// if not found add tag :(
			else
				result += decoded;	// if found add decoded charachter
		}

		result += rest;
		current_tag.clear();
		current_tag.reserve(6);
	}
	result += current_tag;
	result.shrink_to_fit();
	out = std::move(result);
}