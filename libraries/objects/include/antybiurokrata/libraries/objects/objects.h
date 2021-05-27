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
#include <antybiurokrata/libraries/objects/processing_details.hpp>

namespace core
{
	/** @brief contains objects that should be used for comparing and IO operations */
	namespace objects
	{
		namespace detail
		{
			namespace pd = processing_details;
			using patterns::serial::cser;
			using patterns::serial::dser;
			using patterns::serial::ser;
			using patterns::serial::serial_helper_t;

			/** @brief object representation of ORCID number */
			struct detail_orcid_t : public serial_helper_t
			{
				constexpr static size_t words_in_orcid_num{4ul};
				template<typename T> using fixed_orcid_array = std::array<T, words_in_orcid_num>;
				dser<&detail_orcid_t::_, fixed_orcid_array<uint16_t>> identifier;

				using custom_serialize = pd::collection::serial<fixed_orcid_array, uint16_t>;
				using putter_t			  = pd::collection::array_putter<uint16_t, words_in_orcid_num>;
				using custom_deserialize
					 = pd::collection::deserial<putter_t, fixed_orcid_array, uint16_t>;
				using custom_pretty_print = pd::collection::pretty_print<fixed_orcid_array, uint16_t>;

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
				 * @brief checks is given string is proper as orcid
				 * 
				 * @param data orcid string to check
				 * @return true if string is valid
				 * @return false if string is not valid
				 */
				static bool is_valid_orcid_string(const str_v& data);

				/**
				 * @brief implementation of std::string -> detail_orcid_t conversion
				 * 
				 * @param data orcid string 
				 * @return detail_orcid_t 
				 * @throw assert_exception thrown if string is not valid
				*/
				static detail_orcid_t from_string(const u16str_v& data);

				/**
				 * @brief implementation of std::string -> detail_orcid_t conversion
				 * 
				 * @param data orcid string 
				 * @return detail_orcid_t 
				 * @throw assert_exception thrown if string is not valid
				*/
				static detail_orcid_t from_string(const str_v& data);

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

			/** @brief default unifier, that do nothing */
			struct default_unifier
			{
				u16str& x;
			};

			/** @brief default validator, that does not check anythink */
			struct default_validator
			{
				const u16str_v& x;

				/**
				 * @brief if you write your own validator, override this to indicat is given data ok or not
				 * 
				 * @return true if validation success
				 * @return false otherwise
				 */
				operator bool() const noexcept { return true; }
			};

			/**
			 * @brief wraps string
			 * 
			 * @tparam validator with this struct incoming strings will be validated
			 * @tparam unifier with this struct incoming strings will be unified
			 */
			template<typename validator = default_validator, typename unifier = default_unifier>
			struct detail_string_holder_t : public serial_helper_t
			{
				ser<&detail_string_holder_t::_, u16str> data;
				u16str raw;

				using ser_data_t = decltype(data);
				using inner_t	  = typename ser_data_t::value_type;
				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using custom_serialize	  = pd::string::serial;
				using custom_deserialize  = pd::string::deserial;
				using custom_pretty_print = pd::string::pretty_print;

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

					dassert(validate(v),
							  "incoming data: `"_u16 + u16str{v}
									+ "` cannot be set, it does not meet requirements!"_u16);
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

				/** @brief forward equal operator */
				inline friend bool operator==(const detail_string_holder_t& s1,
														const detail_string_holder_t& s2)
				{
					return s1.data() == s2.data();
				}
				/** @brief forward not equal operator */
				inline friend bool operator!=(const detail_string_holder_t& s1,
														const detail_string_holder_t& s2)
				{
					return !(s1 == s2);
				}
				/** @brief forward less operator */
				inline friend bool operator<(const detail_string_holder_t& s1,
													  const detail_string_holder_t& s2)
				{
					return s1.data() < s2.data();
				}

				// static bool validate(const u16str_v& x) { const bool val = static_cast<bool>(validator{x}); std::cout << "valid: " << val << std::endl; return val; }
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

			/** @brief provides validation for polish names */
			struct polish_validator
			{
				const u16str_v& x;

				operator bool() const noexcept;
			};

			/** @brief provides unification for polish names (capitalizing) */
			struct polish_unifier
			{
				polish_unifier(u16str& x) noexcept;
			};

			/** @brief aliasing for handy usage */
			using polish_name_t = string_holder_custom_t<polish_validator, polish_unifier>;

			constexpr uint8_t max_uint_8_t = std::numeric_limits<uint8_t>::max();

			/** @brief supported id's in parsed json / html documents */
			enum class id_type : uint8_t
			{
				IDT	 = 0,
				DOI	 = 1,
				EISSN	 = 2,
				PISSN	 = 3,
				EID	 = 4,
				WOSUID = 5,
				ISBN	 = 6,

				NOT_FOUND = max_uint_8_t
			};

			/**
			 * @brief provides translation to string for enum: `id_type`
			 */
			struct id_type_translation_unit
			{
				using enum_t		= id_type;
				using base_enum_t = uint8_t;
				inline static const u16str translation[]
					 = {u"IDT", u"DOI", u"EISSN", u"PISSN", u"EID", u"WOSUID", u"ISBN"};
				constexpr static size_t length = sizeof(translation) / sizeof(u16str);
			};

			/** @brief removes special charachters and make everythink uppercase */
			struct ids_unifier
			{
				/** @brief actually does all job */
				ids_unifier(u16str& x) noexcept;
			};

			using ids_string_t			= string_holder_custom_t<default_validator, ids_unifier>;
			using id_type_stringinizer = pd::enums::enum_stringinizer<id_type_translation_unit>;

			/**
			 * @brief wraps map that stores diffrent ids
			 */
			struct detail_ids_storage_t : public serial_helper_t
			{
				pd::map_ser<&serial_helper_t::_, id_type, ids_string_t> data;

				using ser_data_t = decltype(data);
				using inner_t	  = typename ser_data_t::value_type;
				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using custom_serialize	 = pd::map_serial<id_type, ids_string_t>;
				using custom_deserialize = pd::map_deserial<id_type, ids_string_t>;
				using custom_pretty_print
					 = pd::map_pretty_serial<id_type, ids_string_t, id_type_stringinizer>;
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
			using publication_t			= cser<&detail_publication_t::ids>;
			using shared_publication_t = pd::shared_t<publication_t>;

			/**
			 * @brief object representation of memmory-usage-friendly collection with publications
			 */
			struct detail_publications_storage_t : public serial_helper_t
			{
				using inner_t = std::set<shared_publication_t>;
				ser<&serial_helper_t::_, inner_t> data{};

				inner_t* operator->() { return &(data()); }
				const inner_t* operator->() const { return &(data()); }

				using putter_t			  = pd::collection::emplacer<std::set, shared_publication_t>;
				using custom_serialize = pd::collection::serial<std::set, shared_publication_t>;
				using custom_deserialize
					 = pd::collection::deserial<putter_t, std::set, shared_publication_t>;
				using custom_pretty_print
					 = pd::collection::pretty_print<std::set, shared_publication_t>;
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
			using person_t			 = cser<&detail_person_t::publictions>;
			using shared_person_t = processing_details::shared_t<person_t>;


			/**
			 * @brief defines, with data sources will be compared with reference data
			 */
			enum class match_type : uint8_t
			{
				POLSL,
				ORCID,
				SCOPUS,
				NO_MATCH,

				NOT_FOUND = max_uint_8_t
			};

			/**
			 * @brief provides cnoversion from enum to string
			 */
			struct match_type_translation_unit
			{
				using enum_t									  = match_type;
				using base_enum_t								  = uint8_t;
				inline static const u16str translation[] = {u"POLSL", u"ORCID", u"SCOPUS", u"NO_MATCH"};
				constexpr static size_t length			  = sizeof(translation) / sizeof(u16str);
			};

			using ser_match_type = pd::enums::enum_t<match_type_translation_unit>;

			/**
			 * @brief object representation of match
			 */
			struct detail_publication_with_source_t : public serial_helper_t
			{
				dser<&detail_publication_with_source_t::_, ser_match_type> source;
				dser<&detail_publication_with_source_t::source, shared_publication_t> publication;

				/** @brief redirect all comprasion operators */
				inline friend auto operator<=>(const detail_publication_with_source_t& pws1,
														 const detail_publication_with_source_t& pws2)
				{
					return pws1.source()().data() <=> pws2.source()().data();
				}
			};
			using publication_with_source_t = cser<&detail_publication_with_source_t::publication>;


			/**
			 * @brief container with unique values
			 */
			struct detail_sourced_publication_storage_t : serial_helper_t
			{
				using item_t  = publication_with_source_t;
				using inner_t = std::set<item_t>;
				dser<&detail_sourced_publication_storage_t::_, inner_t> data;

				using putter_t				  = pd::collection::emplacer<std::set, item_t>;
				using custom_serialize	  = pd::collection::serial<std::set, item_t>;
				using custom_deserialize  = pd::collection::deserial<putter_t, std::set, item_t>;
				using custom_pretty_print = pd::collection::pretty_print<std::set, item_t>;
			};
			using sourced_publication_storage_t = cser<&detail_sourced_publication_storage_t::data>;

			/**
			 * @brief object representation of comprasion result
			 */
			struct detail_publications_summary_t : public serial_helper_t
			{
				dser<&detail_publications_summary_t::_, shared_publication_t> reference;
				dser<&detail_publications_summary_t::reference, sourced_publication_storage_t> matched;
			};

			using publication_summary_t = cser<&detail_publications_summary_t::matched>;
			using shared_publication_summary_t = pd::shared_t<publication_summary_t>;
		}	 // namespace detail

		using typename detail::id_type;
		using typename detail::match_type;
		using typename detail::orcid_t;
		using typename detail::person_t;
		using typename detail::polish_name_t;
		using publication_summary_t = detail::shared_publication_summary_t;
		using typename detail::publication_t;
		using typename detail::shared_person_t;
		using typename detail::shared_publication_t;
		using typename detail::publication_with_source_t;
	}	 // namespace objects
}	 // namespace core

template<typename stream_type>
inline stream_type& operator<<(stream_type& os, const core::objects::detail::id_type& id)
{
	using patterns::serial::delimiter;
	os << static_cast<int>(id) << delimiter;
	return os;
}

template<typename stream_type>
inline stream_type& operator>>(stream_type& is, core::objects::detail::id_type& id)
{
	int x;
	is >> x;
	patterns::serial::drop_delimiter(is);
	id = static_cast<core::objects::detail::id_type>(x);
	return is;
}