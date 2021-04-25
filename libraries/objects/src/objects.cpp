// STL
#include <ranges>
#include <regex>
#include <charconv>
#include <iomanip>

// Project
#include <antybiurokrata/libraries/objects/objects.h>

template <>
patterns::serial::get_from_stream::get_from_stream<>(std::istream &is, typename core::objects::detail::detail_orcid_t::storage_t &data)
{
	for (size_t i = 0; i < core::objects::detail::detail_orcid_t::words_in_orcid_num; i++)
	{
		is >> data[i];
		is.ignore(1, patterns::serial::delimiter);
	}
}

template <>
patterns::serial::put_to_stream::put_to_stream<>(std::ostream &os, const typename core::objects::detail::detail_orcid_t::storage_t &data)
{
	for (size_t i = 0; i < core::objects::detail::detail_orcid_t::words_in_orcid_num; i++)
		os << data[i] << patterns::serial::delimiter;
}

core::str core::objects::detail::detail_orcid_t::to_string(const core::objects::detail::detail_orcid_t &orcid)
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

bool core::objects::detail::detail_orcid_t::is_valid(const core::str_v &data)
{
	const static std::regex orcid_validator_regex{"(\\d{4}-){3}\\d{4}"};
	return std::regex_match(data.data(), orcid_validator_regex);
}

core::objects::detail::detail_orcid_t core::objects::detail::detail_orcid_t::from_string(const core::str_v &data)
{
	dassert{is_valid(data), "given string is not valid ORCID number"};
	const std::ranges::split_view splitter{data, '-'};
	detail_orcid_t result{};

	size_t i = 0;
	for (const auto &part : splitter)
	{
		str x;
		x.reserve(std::ranges::distance(part));
		for (const auto c : part)
			x += c;
		result.identifier()[i] = static_cast<uint16_t>(std::stoi(x));
		i++;
	}

	return result;
}

core::objects::detail::detail_string_holder_t::detail_string_holder_t(const core::u16str_v& v)
{
	using namespace core;
	if(v.size() == 0) return;

	const bool has_hashes{ v.find(u'#') != u16str_v::npos };
	const bool has_ampersands{ v.find(u'&') != u16str_v::npos };

	if(has_hashes)
	{
		if(has_ampersands) demangler<>::mangle<conv_t::HTML>(data());
		else demangler<>::mangle<conv_t::URL>(data());
	}else data(v);
}