/**
 * @file objects.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Here are placed most common classes, structs and concepts
 * 
 * @copyright Copyright (c) 2021
 * 
*/

#pragma once

// Project
#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <antybiurokrata/libraries/demangler/demangler.h>

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

				/** @brief default constructor */
				explicit detail_orcid_t() = default;

				/**
				 * @brief Construct a new detail orcid t object
				 * 
				 * @param input valid orcid as string_view
				*/
				explicit detail_orcid_t(const str_v& input) : detail_orcid_t{ std::move(from_string(input)) } {}

				/**
				 * @brief provides easy conversion to string
				 * 
				 * @return str
				*/
				explicit operator str() const { return to_string(*this); }

				/**
				 * @brief implementation of detail_orcid_t -> std::string conversion
				 * 
				 * @param orcid object to convert
				 * @return str
				 */
				static str to_string(const detail_orcid_t& orcid);

				/**
				 * @brief checks is given string is proper as orcid
				 * 
				 * @param data orcid string to check
				 * @return true if string is valid
				 * @return false if string is not valid
				 */
				static bool is_valid(const str_v& data);

				/**
				 * @brief implementation of std::string -> detail_orcid_t conversion
				 * 
				 * @param data orcid string 
				 * @return detail_orcid_t 
				 * @throw assert_exception thrown if string is not valid
				*/
				static detail_orcid_t from_string(const str_v& data);
			};
			using orcid_t = cser<&detail_orcid_t::identifier>;

			struct detail_string_holder_t : public serial_helper_t
			{
				ser<&detail_string_holder_t::_,	u16str> data;

				/** brief default constructor */
				detail_string_holder_t() = default;

				/** @brief 1) Construct a new detail detail_string_holder_t object from string view; forwards to 2 */
				explicit detail_string_holder_t(const str_v& v) : detail_string_holder_t{ str{v.data()} } {}

				/** @brief 2) Construct a new detail detail_string_holder_t object from string; forwards to 3 */
				explicit detail_string_holder_t(const str& v) : detail_string_holder_t{ core::get_conversion_engine().from_bytes(v) } {}

				/** @brief 3) Construct a new detail detail_string_holder_t object from u16string_view */
				explicit detail_string_holder_t(const u16str_v& v);

				/** @brief 4) Construct a new detail detail_string_holder_t object from u16string; forwards to 3 */
				explicit detail_string_holder_t(const u16str& v) : detail_string_holder_t{ u16str_v{v} } {}

				/** @brief provides conversion to string*/
				explicit operator str() const;

				/** @brief provides conversion to u16string_view*/
				explicit operator u16str_v() const { return u16str_v{ data() }; }

				/**
				 * @brief provides conversion easy conversion with demangler
				 * 
				 * @tparam conv_t type of conversion
				 * @return str 
				*/
				template<core::detail::conversion_t conv_t>
				str get_as() const { return core::demangler<str, str_v>{ static_cast<str>(*this) }.process<conv_t>().get_copy(); }

				/**
				 * @brief provides conversion easy conversion with demangler
				 * 
				 * @tparam conv_t type of conversion
				 * @return u16str 
				*/
				template<core::detail::conversion_t conv_t>
				u16str get_as() const { return core::demangler<u16str, u16str_v>{ data() }.process<conv_t>().get_copy(); }
			};

			/** @brief object representation and holder of polish name */
			struct detail_polish_name_t : public detail_string_holder_t
			{
				constexpr static str_v msg_not_valid{ "given string is not valid for polish name" };

				/** @brief default constructor */
				// detail_polish_name_t() = default;

				/**
				 * @brief forward all to parent constructor
				 * 
				 * @tparam string_type basically everythink that can construct str or u16str
				 */
				template<typename string_type>
				requires(!std::is_same_v<string_type, ___null_t>)
				explicit detail_polish_name_t(string_type&& v) : detail_string_holder_t{std::forward<string_type>(v)} { validate(); unify(); }

				/** @brief in this case it's proxy to toupper */
				void unify() noexcept;

			protected:

				/** @brief override this if you dervie from this class, it's guaranteed that data is set */
				[[nodiscard]] virtual bool is_valid() const;

				/** 
				 * @brief if validation fails it's throw assertion
				 * 
				 * @throw assert_exception if given string is not valid
				 */
				void validate() const { dassert{ is_valid(), msg_not_valid }; }
			};
			using polish_name_t = cser<&detail_polish_name_t::data>;

			struct detail_person_t : public serial_helper_t
			{
				ser<&detail_person_t::_,		polish_name_t>	name;
				ser<&detail_person_t::name,		polish_name_t>	surname;
				ser<&detail_person_t::surname,	orcid_t>		orcid;
			};
			using person_t = cser<&detail_person_t::orcid>;
		}

		using typename detail::orcid_t;
		using typename detail::polish_name_t;
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

