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
#include <antybiurokrata/libraries/demangler/demangler.h>
#include <antybiurokrata/libraries/objects/serialization_definitions.hpp>

namespace core
{
	namespace objects
	{
		namespace detail
		{
			using namespace serial_definitions;

			/** @brief object representation of ORCID number */
			struct detail_orcid_t : public serial_helper_t
			{
				constexpr static size_t words_in_orcid_num{ 4ul };

				array_ser<&detail_orcid_t::_, uint16_t, words_in_orcid_num> identifier;

				/** @brief default constructor */
				explicit detail_orcid_t() = default;

				/**
				 * @brief Construct a new detail orcid t object
				 * 
				 * @param input valid orcid as string_view
				*/
				explicit detail_orcid_t(const u16str_v& input) : detail_orcid_t{ std::move(from_string(input)) } {}

				/**
				 * @brief provides easy conversion to u16str
				 * 
				 * @return u16str
				*/
				explicit operator u16str() const { return get_conversion_engine().from_bytes(to_string(*this)); }

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
				 * @param conversion_output [ = nullptr ] during validation, conversion is performed, it you need it further, conversion output will be saved here
				 * @return true if string is valid
				 * @return false if string is not valid
				 */
				static bool is_valid(const u16str_v& data, str* conversion_output = nullptr);

				/**
				 * @brief implementation of std::string -> detail_orcid_t conversion
				 * 
				 * @param data orcid string 
				 * @return detail_orcid_t 
				 * @throw assert_exception thrown if string is not valid
				*/
				static detail_orcid_t from_string(const u16str_v& data);

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

				template<typename U>
				detail_string_holder_t& operator=(U&& u) { detail_string_holder_t x{u}; this->data(x.data()); return *this; }

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

				inline friend bool operator==(const detail_string_holder_t& s1, const detail_string_holder_t& s2) { return s1.data() == s2.data(); }
				inline friend bool operator!=(const detail_string_holder_t& s1, const detail_string_holder_t& s2) { return !(s1 == s2); }
			};
			using string_holder_t = cser<&detail_string_holder_t::data>;

			/** @brief object representation and holder of polish name */
			struct detail_polish_name_t : public serial_helper_t
			{
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

				template<typename U>
				detail_polish_name_t& operator=(U&& u) { detail_polish_name_t x(std::forward<U>(u)); data()().data() = x.data()().data(); return *this; }

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
				void validate() const { dassert{ is_valid(), static_cast<str>(data()()) + ": is not valid for polish name"}; }

			};
			using polish_name_t = cser<&detail_polish_name_t::data>;

			enum class id_type : uint8_t
			{
				IDT = 0,
				DOI = 1,
				EISSN = 2,
				PISSN = 3
			};

			/** @brief object representation of publication */
			struct detail_publication_t : public serial_helper_t
			{
				u16ser<		&detail_publication_t::_>								title;
				u16ser<		&detail_publication_t::title>							polish_title;
				dser<		&detail_publication_t::polish_title, uint16_t>			year;

				using ids_map_t = map_ser<	&detail_publication_t::year, id_type, string_holder_t, enum_printer<id_type> >;
				ids_map_t 															ids;

				bool compare(const detail_publication_t&) const;
				inline friend bool operator==(const detail_publication_t& me, const detail_publication_t& other) { return me.compare(other); }
				inline friend bool operator!=(const detail_publication_t& me, const detail_publication_t& other) { return !(me == other); }
			};
			using publication_t = cser<&detail_publication_t::ids>;

			/** @brief object representation of person (author) */
			struct detail_person_t : public serial_helper_t
			{
				dser<		&detail_person_t::_,		polish_name_t>				name;
				dser<		&detail_person_t::name,		polish_name_t>				surname;
				dser<		&detail_person_t::surname,	orcid_t>					orcid;
				svec_ser<	&detail_person_t::orcid,	publication_t>				publictions{};

				friend inline bool operator==(const detail_person_t& p1, const detail_person_t& p2) 
				{
					if(p1.orcid() == p2.orcid()) return true;
					else return ( p1.name() == p2.name() ) && ( p1.surname() == p2.surname() );
				}

				friend inline bool operator<(const detail_person_t& p1, const detail_person_t& p2)
				{
					return static_cast<u16str_v>(p1.surname()()) < static_cast<u16str_v>(p2.surname()());
				}
			};
			using person_t = cser<&detail_person_t::publictions>;
		}

		using typename detail::orcid_t;
		using typename detail::polish_name_t;
		using typename detail::publication_t;
		using typename detail::person_t;
	}
}

template<typename stream_type>
inline stream_type& operator<<(stream_type& os, const core::objects::detail::id_type& id)
{
	return os << static_cast<int>(id);
}

template<typename stream_type>
inline stream_type& operator>>(stream_type& is, core::objects::detail::id_type& id)
{
	int x;
	is >> x;
	id = static_cast<core::objects::detail::id_type>(x);
	return is;
}