/**
 * @file objects.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Here are placed most common classes, structs and concepts
 * 
 * @copyright Copyright (c) 2021
 * 
*/

#pragma once

#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace objects
	{
		template<typename X>
		concept iterable_req = requires(X x)
		{
			{ x.begin() };
			{ x.end() };
			// { x.capacity() } -> std::same_as<size_t>;s
			{ x.begin()++ };
			{ *(x.begin()) };
		};

		namespace detail
		{
			using namespace patterns::serial;
			struct detail_orcid_t : public serial_helper_t
			{
				constexpr static size_t words_in_orcid_num{ 4ul };
				using storage_t = std::array<uint16_t, words_in_orcid_num>;

				ser<&detail_orcid_t::_,	storage_t> identifier;
			};
			using orcid_t = cser<&detail_orcid_t::identifier>;

			struct detail_person_t : public serial_helper_t
			{
				ser<&detail_person_t::_,		str>		name;
				ser<&detail_person_t::name,		str>		surname;
				ser<&detail_person_t::surname,	orcid_t>	orcid;
			};
			using person_t = cser<&detail_person_t::orcid>;
		}

		using typename detail::orcid_t;
		using typename detail::person_t;
	}
}

template<> 
patterns::serial::get_from_stream::get_from_stream<>
(std::istream& is, typename core::objects::detail::detail_orcid_t::storage_t& data)
{
	for(size_t i = 0; i < core::objects::detail::detail_orcid_t::words_in_orcid_num; i++)
	{
		is >> data[i];
		is.ignore(1, patterns::serial::delimiter);
	}
}

// template<core::objects::iterable_req Iterable>
// requires // exclude strings
// (
// 	!std::is_same_v<Iterable, core::str> and
// 	!std::is_same_v<Iterable, core::u16str>
// )
// inline std::ostream& operator<<(std::ostream& os, const Iterable& iterable)
// {
// 	for(const auto& x : iterable) os << x << patterns::serial::delimiter;
// 	return os;
// }

// template<core::objects::iterable_req Iterable>
// requires // exclude strings
// (
// 	!std::is_same_v<Iterable, core::str> and
// 	!std::is_same_v<Iterable, core::u16str>
// )
// inline std::istream& operator<<(std::istream& is, const Iterable& iterable)
// {
// 	const size_t cap = iterable.capacity();
// 	for(size_t i = 0; i < cap; ++i) is >> iterable;
// 	return is;
// }

