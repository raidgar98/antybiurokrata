/**
 * @file objects.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Here are placed most common classes, structs and concepts
 * 
 * @copyright Copyright (c) 2021
 * 
*/

#pragma once

// STL
#include <ranges>
#include <regex>
#include <charconv>
#include <iomanip>

// Project
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <antybiurokrata/types.hpp>


template<> patterns::serial::put_to_stream::put_to_stream<> (std::ostream& os, const std::array<uint16_t, 4>& data);
template<> patterns::serial::get_from_stream::get_from_stream<> (std::istream& is, std::array<uint16_t, 4>& data);

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

			/** @brief object representation of ORCID number */
			struct detail_orcid_t : public serial_helper_t
			{
				constexpr static size_t words_in_orcid_num{ 4ul };
				template<size_t N>
				requires serializable_req<std::array<uint16_t, N>>
				using n_storage_t = std::array<uint16_t, words_in_orcid_num>;

				using storage_t = n_storage_t<words_in_orcid_num>;

				ser<&detail_orcid_t::_,	storage_t> identifier;

				explicit detail_orcid_t() = default;
				explicit detail_orcid_t(const str_v& input) : detail_orcid_t{ std::move(from_string(input)) } {}

				explicit operator str() const
				{ 
					std::stringstream result;
					for(auto it = this->identifier().begin(); it != this->identifier().end(); it++)
					{
						if(it != this->identifier().begin()) result << std::setw(1) << '-';
						result << std::setfill('0') << std::setw(4) << std::to_string(*it);
					}
					return result.str();
				}

				static bool is_valid(const str_v& data)
				{
					const static std::regex orcid_validator_regex{ "(\\d{4}-){3}\\d{4}" };
					return std::regex_match( data.data() , orcid_validator_regex );
				}

				static detail_orcid_t from_string(const str_v& data)
				{
					dassert{ is_valid(data), "given string is not valid ORCID number" };
					const std::ranges::split_view splitter{ data, '-' };
					detail_orcid_t result{};

					size_t i = 0;
					for(const auto& part : splitter)
					{
						str x; x.reserve( std::ranges::distance(part) );
						for(const auto c : part) x += c;
						result.identifier()[i] = static_cast<uint16_t>( std::stoi(x) );
						i++;
					}

					return result;
				}
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

