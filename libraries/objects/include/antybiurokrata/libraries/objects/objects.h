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

#pragma todo "refactor to u16str_v and u16str!!!"

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
			using patterns::serial::serial_helper_t;

			template<typename T, size_t N>
			struct array_serial
			{
				template<typename stream_type>
				array_serial(stream_type& os, const std::array<T, N>& data)
				{
					using patterns::serial::delimiter;
					for (size_t i = 0; i < N; i++)
						os << data[i] << delimiter;
				}
			};

			template<typename T, size_t N>
			struct array_deserial
			{
				template<typename stream_type>
				array_deserial(stream_type& is, std::array<T, N>& data)
				{
					using patterns::serial::delimiter;
					for (size_t i = 0; i < N; i++)
					{
						is >> data[i];
						is.ignore(1, delimiter);
					}
				}
			};

			/** @brief object representation of ORCID number */
			struct detail_orcid_t : public serial_helper_t
			{
				constexpr static size_t words_in_orcid_num{ 4ul };
				template<size_t N>
				requires serializable_req<std::array<uint16_t, N>>
				using n_storage_t = std::array<uint16_t, words_in_orcid_num>;
				using storage_t = n_storage_t<words_in_orcid_num>;

				ser<&detail_orcid_t::_,	storage_t, array_serial<uint16_t, words_in_orcid_num>, array_deserial<uint16_t, words_in_orcid_num>> identifier;

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

				friend inline bool operator==(const detail_orcid_t& o1, const detail_orcid_t& o2) { return o1.identifier() == o2.identifier(); }
			};
			using orcid_t = cser<&detail_orcid_t::identifier>;

			struct detail_string_holder_t : public serial_helper_t
			{
				u16ser<&detail_string_holder_t::_> data;

				/** brief default constructor */
				detail_string_holder_t() = default;
				detail_string_holder_t(const detail_string_holder_t&) = default;
				detail_string_holder_t(detail_string_holder_t&&) = default;

				/** @brief 1) Construct a new detail detail_string_holder_t object from string view; forwards to 2 */
				explicit detail_string_holder_t(const str_v& v) : detail_string_holder_t{ str{v.data()} } 
				{}

				/** @brief 2) Construct a new detail detail_string_holder_t object from string; forwards to 3 */
				explicit detail_string_holder_t(const str& v) : detail_string_holder_t{ core::get_conversion_engine().from_bytes(v) } 
				{}

				/** @brief 3) Construct a new detail detail_string_holder_t object from u16string_view */
				explicit detail_string_holder_t(const u16str_v& v);

				/** @brief 4) Construct a new detail detail_string_holder_t object from u16string; forwards to 3 */
				explicit detail_string_holder_t(const u16str& v) : detail_string_holder_t{ u16str_v{v} } 
				{}

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
			using string_holder_t = cser<&detail_string_holder_t::data>;

			/** @brief object representation and holder of polish name */
			struct detail_polish_name_t : public serial_helper_t
			{
				constexpr static str_v msg_not_valid{ "given string is not valid for polish name" };
				dser<&detail_polish_name_t::_, string_holder_t> data;

				/** @brief default constructor */
				detail_polish_name_t() = default;

				/**
				 * @brief forward all to parent constructor
				 * 
				 * @tparam string_type basically everythink that can construct str or u16str
				 */
				template<typename ... U>
				// requires(!std::is_same_v< std::tuple_element_t<0, std::tuple<U...>>, ___null_t>)
				detail_polish_name_t(U&& ... v) : data{std::forward<U>(v)...} { validate(); unify(); }

				/** @brief in this case it's proxy to toupper */
				void unify() noexcept;

				/** @brief forwarding to data*/
				explicit operator str() const { return data()().operator core::str(); }

				/** @brief forwarding to data*/
				explicit operator u16str_v() const { return data()().operator u16str_v(); }

				friend inline bool operator==(const detail_polish_name_t& pn1, const detail_polish_name_t& pn2) { return pn1.data()().data() == pn2.data()().data(); }

				[[nodiscard]] static bool basic_validation(u16str_v input);

			protected:

				/** @brief override this if you dervie from this class, it's guaranteed that data is set */
				[[nodiscard]] virtual bool is_valid() const;

				/** 
				 * @brief if validation fails it's throw assertion
				 * 
				 * @throw assert_exception if given string is not valid
				 */
				void validate() const { dassert{ is_valid(), msg_not_valid}; }

			};
			using polish_name_t = cser<&detail_polish_name_t::data>;

			struct detail_person_t : public serial_helper_t
			{
				dser<&detail_person_t::_,		polish_name_t>	name;
				dser<&detail_person_t::name,	polish_name_t>	surname;
				dser<&detail_person_t::surname,	orcid_t>		orcid;

				friend inline bool operator==(const detail_person_t& p1, const detail_person_t& p2) 
				{
					if(p1.orcid() == p2.orcid()) return true;
					else return ( p1.name() == p2.name() ) && ( p1.surname() == p2.surname() );
				}
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

