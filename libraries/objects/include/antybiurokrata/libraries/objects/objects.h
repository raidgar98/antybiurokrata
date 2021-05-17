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
				constexpr static size_t words_in_orcid_num{4ul};
				array_ser<&detail_orcid_t::_, uint16_t, words_in_orcid_num> identifier;

				using custom_serialize	  = array_serial<uint16_t, words_in_orcid_num>;
				using custom_deserialize  = array_deserial<uint16_t, words_in_orcid_num>;
				using custom_pretty_print = array_pretty_serial<uint16_t, words_in_orcid_num>;

				/** @brief default constructor */
				explicit detail_orcid_t() = default;

				/**
				 * @brief Construct a new detail orcid t object
				 * 
				 * @param input valid orcid as string_view
				*/
				detail_orcid_t(const u16str_v& input) : detail_orcid_t{std::move(from_string(input))} {}

				/**
				 * @brief provides easy conversion to u16str
				 * 
				 * @return u16str
				*/
				operator u16str() const { return get_conversion_engine().from_bytes(to_string(*this)); }

				/**
				 * @brief provides easy conversion to string
				 * 
				 * @return str
				*/
				operator str() const { return to_string(*this); }

				/**
				 * @brief checks is current orcid is proper (if 0000-0000-0000-0000 it's not)
				 * 
				 * @return true if so
				 * @return false if not
				 */
				bool is_valid_orcid() const;

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
				static bool is_valid_orcid_string(const u16str_v& data,
															 str* conversion_output = nullptr);

				/**
				 * @brief implementation of std::string -> detail_orcid_t conversion
				 * 
				 * @param data orcid string 
				 * @return detail_orcid_t 
				 * @throw assert_exception thrown if string is not valid
				*/
				static detail_orcid_t from_string(const u16str_v& data);

				friend inline bool operator==(const detail_orcid_t& o1, const detail_orcid_t& o2)
				{
					return o1.identifier() == o2.identifier();
				}
				friend inline bool operator!=(const detail_orcid_t& o1, const detail_orcid_t& o2)
				{
					return !(o1 == o2);
				}
				friend inline bool operator<(const detail_orcid_t& o1, const detail_orcid_t& o2)
				{
					for(size_t i = 0; i < detail_orcid_t::words_in_orcid_num; ++i)
						if(o1.identifier()[i] != o2.identifier()[i])
							return o1.identifier()[i] < o2.identifier()[i];

					return false;	 // they are same
				}
			};
			using orcid_t = cser<&detail_orcid_t::identifier>;


			struct default_unifier
			{
				u16str& x;
			};
			struct default_validator
			{
				const u16str_v& x;
				operator bool() const noexcept { return true; }
			};

			template<typename validator = default_validator, typename unifier = default_unifier>
			struct detail_string_holder_t : public serial_helper_t
			{
				dser<&detail_string_holder_t::_, u16str> data;
				u16str raw;

				using ser_data_t = decltype(data);
				using inner_t	  = typename ser_data_t::value_type;
				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using custom_serialize	  = u16str_serial;
				using custom_deserialize  = u16str_deserial;
				using custom_pretty_print = u16str_pretty_serial;

				/** @brief default constructor */
				detail_string_holder_t()										= default;
				detail_string_holder_t(const detail_string_holder_t&) = default;
				detail_string_holder_t(detail_string_holder_t&&)		= default;
				detail_string_holder_t& operator=(const detail_string_holder_t&) = default;
				detail_string_holder_t& operator=(detail_string_holder_t&&) = default;

				/** @brief this is constructor body, preferable way of setting data */
				void set(const u16str_v& v)
				{
					using namespace core;

					if(v.size() == 0) return;
					const bool has_hashes{v.find(u'#') != u16str_v::npos};
					const bool has_ampersands{v.find(u'&') != u16str_v::npos};
					const bool has_percents{v.find(u'%') != u16str_v::npos};
					data(v);

					if(has_hashes && has_ampersands) demangler<>::mangle<conv_t::HTML>(data());
					else if(has_percents)
						demangler<>::mangle<conv_t::URL>(data());

					raw = v;
					unifier{data()};
				}

				/** @brief 1) Construct a new detail detail_string_holder_t object from string view; forwards to 2 */
				explicit detail_string_holder_t(const str_v& v) : detail_string_holder_t{str{v.data()}}
				{
				}

				/** @brief 2) Construct a new detail detail_string_holder_t object from string; forwards to 3 */
				explicit detail_string_holder_t(const str& v) :
					 detail_string_holder_t{core::get_conversion_engine().from_bytes(v)}
				{
				}

				/** @brief 3) Construct a new detail detail_string_holder_t object from u16string_view */
				explicit detail_string_holder_t(const u16str_v& v) { set(v); }

				/** @brief 4) Construct a new detail detail_string_holder_t object from u16string; forwards to 3 */
				explicit detail_string_holder_t(const u16str& v) : detail_string_holder_t{u16str_v{v}}
				{
				}

				/** @brief 1) forward to assign operator 2 */
				detail_string_holder_t& operator=(const str_v& v) { return (*this = str{v.data()}); }

				/** @brief 2) forward to assign operator 3 */
				detail_string_holder_t& operator=(const str& v)
				{
					return (*this = core::get_conversion_engine().from_bytes(v));
				}

				/** @brief 3) actually constructs object */
				detail_string_holder_t& operator=(const u16str_v& v)
				{
					this->set(v);
					return *this;
				}

				/** @brief 4) forward to assign operator 3 */
				detail_string_holder_t& operator=(const u16str& v) { return (*this = u16str_v{v}); }

				/** @brief provides conversion to u16string_view*/
				operator u16str_v() const { return u16str_v{data()}; }

				/**
				 * @brief provides conversion easy conversion with demangler
				 * 
				 * @tparam conv_t type of conversion
				 * @return u16str 
				*/
				template<core::detail::conversion_t conv_t> u16str get_as() const
				{
					return core::demangler<u16str, u16str_v>{data()}.process<conv_t>().get_copy();
				}

				inline friend bool operator==(const detail_string_holder_t& s1,
														const detail_string_holder_t& s2)
				{
					return s1.data() == s2.data();
				}
				inline friend bool operator!=(const detail_string_holder_t& s1,
														const detail_string_holder_t& s2)
				{
					return !(s1 == s2);
				}
				inline friend bool operator<(const detail_string_holder_t& s1,
													  const detail_string_holder_t& s2)
				{
					return s1.data() < s2.data();
				}

				static bool validate(const u16str_v& x) { return static_cast<bool>(validator{x}); }

			 private:
				/** 
				 * @brief if validation fails it's throw assertion
				 * 
				 * @throw assert_exception if given string is not valid
				 */
				void validate() const { dassert{validate(data()), data() + ": is not valid name"_u16}; }
			};
			template<typename validator = default_validator, typename unifier = default_unifier>
			using string_holder_custom_t	= cser<&detail_string_holder_t<validator, unifier>::data>;
			using string_holder_t			= string_holder_custom_t<>;
			template<auto X> using u16ser = dser<X, string_holder_t>;

			struct polish_validator
			{
				const u16str_v& x;

				operator bool() const noexcept;
			};

			struct polish_unifier
			{
				polish_unifier(u16str& x) noexcept;
			};

			using polish_name_t = string_holder_custom_t<polish_validator, polish_unifier>;

			enum class id_type : uint8_t
			{
				IDT	 = 0,
				DOI	 = 1,
				EISSN	 = 2,
				PISSN	 = 3,
				EID	 = 4,
				WOSUID = 5,
				ISBN	 = 6,

				NOT_FOUND = 99
			};

			struct id_type_stringinizer
			{
				inline static const u16str enum_to_string[]
					 = {u"IDT", u"DOI", u"EISSN", u"PISSN", u"EID", u"WOSUID", u"ISBN"};
				constexpr static size_t length{sizeof(enum_to_string) / sizeof(str)};

				const id_type id;

				static u16str get(const id_type x)
				{
					const size_t index = static_cast<size_t>(x);
					dassert(index < length, "invalid id_type"_u8);
					return id_type_stringinizer::enum_to_string[index];
				}

				static id_type get(u16str x)
				{
					std::for_each(x.begin(), x.end(), [](u16char_t& c) { c = std::toupper(c); });
					for(size_t i = 0; i < length; ++i)
						if(x == enum_to_string[i]) return static_cast<id_type>(i);
					// dassert(false, "invalid string: "_u16 + x);
					global_logger.warn() << "invalid string: " << x << logger::endl;
					return id_type::NOT_FOUND;
				}

				template<typename stream_t>
				inline friend stream_t& operator<<(stream_t& os, const id_type_stringinizer& x)
				{
					return os << get_conversion_engine().to_bytes(get(x.id));
				}
			};

			struct ids_unifier
			{
				ids_unifier(u16str& x) noexcept;
			};

			using ids_string_t = string_holder_custom_t<default_validator, ids_unifier>;

			struct detail_ids_storage_t : public serial_helper_t
			{
				map_ser<&serial_helper_t::_, id_type, ids_string_t> data;

				using ser_data_t = decltype(data);
				using inner_t	  = typename ser_data_t::value_type;
				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using custom_serialize	 = map_serial<id_type, ids_string_t>;
				using custom_deserialize = map_deserial<id_type, ids_string_t>;
				using custom_pretty_print
					 = map_pretty_serial<id_type, ids_string_t, id_type_stringinizer>;
			};
			using ids_storage_t = cser<&detail_ids_storage_t::data>;

			/** @brief object representation of publication */
			struct detail_publication_t : public serial_helper_t
			{
				u16ser<&detail_publication_t::_> title;
				u16ser<&detail_publication_t::title> polish_title;
				dser<&detail_publication_t::polish_title, uint16_t> year;
				dser<&detail_publication_t::year, ids_storage_t> ids;

				/**
				 * @brief compares two me with other
				 * 
				 * @return int 0 = equal, 1 = greate, -1 = lesser
				 */
				int compare(const detail_publication_t&) const;
				inline friend bool operator==(const detail_publication_t& me,
														const detail_publication_t& other)
				{
					return me.compare(other) == 0;
				}
				inline friend bool operator!=(const detail_publication_t& me,
														const detail_publication_t& other)
				{
					return !(me == other);
				}

				inline friend bool operator<(const detail_publication_t& me,
														const detail_publication_t& other)
				{
					return me.compare(other) < 0;
				}

				inline friend bool operator>(const detail_publication_t& me,
														const detail_publication_t& other)
				{
					return me.compare(other) > 0;
				}
			};
			using publication_t = cser<&detail_publication_t::ids>;

			struct detail_publications_storage_t : public serial_helper_t
			{
				sset_ser<&serial_helper_t::_, publication_t> data{};

				using ser_data_t = decltype(data);
				using inner_t	  = typename ser_data_t::value_type;
				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using custom_serialize	  = shared_set_serial<publication_t>;
				using custom_deserialize  = shared_set_deserial<publication_t>;
				using custom_pretty_print = shared_set_pretty_serial<publication_t>;
			};
			using publications_storage_t = cser<&detail_publications_storage_t::data>;

			/** @brief object representation of person (author) */
			struct detail_person_t : public serial_helper_t
			{
				dser<&detail_person_t::_, polish_name_t> name;
				dser<&detail_person_t::name, polish_name_t> surname;
				dser<&detail_person_t::surname, orcid_t> orcid;
				mutable dser<&detail_person_t::orcid, publications_storage_t> publictions{};

				friend inline bool operator==(const detail_person_t& p1, const detail_person_t& p2)
				{
					if(p1.orcid()() == p2.orcid()()) [[likely]]
						return true;
					else
						return (p1.name()() == p2.name()()) && (p1.surname()() == p2.surname()());
				}
				friend inline bool operator!=(const detail_person_t& p1, const detail_person_t& p2)
				{
					return !(p1 == p2);
				}

				friend inline bool operator<(const detail_person_t& p1, const detail_person_t& p2)
				{
					if(p1.orcid()().is_valid_orcid() && p2.orcid()().is_valid_orcid()) [[likely]]
						return p1.orcid()() < p2.orcid()();
					else if(p1.surname()() != p2.surname()())
						return p1.surname()() < p2.surname()();
					else
						return p1.name()() < p2.name()();
				}
			};
			using person_t = cser<&detail_person_t::publictions>;
		}	 // namespace detail

		using typename detail::id_type;
		using typename detail::orcid_t;
		using typename detail::person_t;
		using typename detail::polish_name_t;
		using typename detail::publication_t;
	}	 // namespace objects
}	 // namespace core

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