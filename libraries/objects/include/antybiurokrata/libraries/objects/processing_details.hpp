/**
 * @file serialization_definitions.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contain serialization definitions for more complex types
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <iomanip>
#include <array>
#include <set>
#include <map>

namespace core
{
	namespace objects
	{
		/**
		 * @brief templates of actions on complex types
		 */
		namespace processing_details
		{
			using namespace patterns::serial;
			using patterns::serial::drop_delimiter;
			using patterns::serial::serial_helper_t;

			/** @brief contains custom processing options for std::shared_ptr<T> processing */
			namespace shared
			{
				/**
				 * @brief compares to shared_pointers
				 * 
				 * @tparam T any wrapped type by shared_pointer
				 */
				template<typename T> struct compare
				{
					using sT = std::shared_ptr<T>;
					bool operator()(const sT& s1, const sT& s2) const
					{
						check_nullptr{s1};
						check_nullptr{s2};
						return *s1 < *s2;
					}
				};

				/**
				 * @brief serializes object under shared_ptr
				 * 
				 * @tparam T any wrapped type by shared_pointer
				 */
				template<typename T> struct serial
				{
					using sT = std::shared_ptr<T>;
					template<typename stream_type> serial(stream_type& os, const sT& ptr)
					{
						using patterns::serial::delimiter;
						const bool is_null = ptr.get() == nullptr;
						os << static_cast<int>(is_null) << delimiter;
						if(!is_null) os << *ptr << delimiter;
					}
				};

				/**
				 * @brief deserializes input stream to object under given shared_ptr
				 * 
				 * @tparam T any wrapped type by shared_pointer
				 */
				template<typename T> struct deserial
				{
					using sT = std::shared_ptr<T>;
					template<typename stream_type> deserial(stream_type& is, sT& ptr)
					{
						int is_null;
						is >> is_null;
						drop_delimiter(is);

						if(is_null != 0)
						{
							ptr = std::make_shared<T>();
							is >> *ptr;
							drop_delimiter(is);
						}
					}
				};

				/**
				 * @brief pretty prints value under pointer
				 * 
				 * @tparam T any wrapped type by shared_pointer
				 */
				template<typename T> struct pretty_print
				{
					using sT = std::shared_ptr<T>;
					constexpr static str_v null_value_string{"NULL"};

					template<typename stream_type> pretty_print(stream_type& os, sT& ptr)
					{
						if(ptr) os << patterns::serial::pretty_print{*ptr};
						else
							os << null_value_string;
					}
				};

				namespace ps = patterns::serial;

				/**
				 * @brief helper crass to quickly create shared single member classes
				 * 
				 * @tparam T any type to encapsulate
				 */
				template<typename T>
				struct detail_single_member_shared_struct_helper : public ps::serial_helper_t
				{
					using value_type = std::shared_ptr<T>;
					using my_t		  = detail_single_member_shared_struct_helper;
					ps::dser<&my_t::_, value_type> data{new T{}};

					using custom_serialize	  = shared::serial<T>;
					using custom_deserialize  = shared::deserial<T>;
					using custom_pretty_print = shared::pretty_print<T>;

					/** @brief quick access to pointer */
					T* operator->()
					{
						valid();
						return data().get();
					}

					/** @brief quick access to pointer */
					const T* operator->() const
					{
						valid();
						return data().get();
					}

					T& operator*()
					{
						valid();
						return *data().get();
					}

					const T& operator*() const
					{
						valid();
						return *data().get();
					}

					/** @brief proxies, to make this class usable as much as shared_ptr */
					operator bool() const { return validate(); }

					/** @brief returns true if pointer is set */
					bool validate() const { return this->data().get() != nullptr; }

					/** @brief forwarding less operator */
					inline friend bool operator<(const my_t& s1, const my_t& s2)
					{
						return shared::compare<T>{}(s1.data(), s2.data());
					}

				 private:
					/** @brief checks is pointer not null */
					void valid() const { check_nullptr{data()}; }
				};

				template<typename T>
				using single_member_shared_struct_helper
					 = ps::cser<&detail_single_member_shared_struct_helper<T>::data>;
			}	 // namespace shared

			/** @brief contatins custom processing options for u16str */
			namespace string
			{
				/**
				 * @brief serialization of wide string
				 */
				struct serial
				{
					template<typename stream_type> serial(stream_type& os, const u16str_v& view)
					{
						using patterns::serial::delimiter;
						os << view.size() << delimiter;
						for(const auto c: view) os << static_cast<int>(c) << delimiter;
					}
				};

				/**
				 * @brief deserialization of wide string
				 */
				struct deserial
				{
					template<typename stream_type> deserial(stream_type& is, typename core::u16str& out)
					{
						size_t size;
						is >> size;
						drop_delimiter(is);
						if(size == 0) return;
						else
							out.reserve(size);
						for(size_t i = 0; i < size; ++i)
						{
							int c;
							is >> c;
							drop_delimiter(is);
							out += static_cast<u16char_t>(c);
						}
					}
				};

				/** @brief pretty-printing of wide stirng (just proxy to conversion) */

				struct pretty_print
				{
					template<typename stream_type> pretty_print(stream_type& os, const u16str_v& view)
					{
						os << get_conversion_engine().to_bytes(view.data());
					}
				};
			}	 // namespace string

			namespace collection
			{
				/**
				 * @brief defines minimum requirements for iterator
				 * 
				 * @tparam iterator_t type to check
				 */
				template<typename iterator_t> concept iterator_req = requires(iterator_t it)
				{
					{*it};
					{it++};
					{std::distance(it, it)};
				};

				/**
				 * @brief defines minimum requirements for colletion
				 * 
				 * @tparam coll_t type to check
				 */
				template<template<typename T, typename... argv> typename coll_t, typename elem_t,
							typename... args>
				concept iterable_req = requires(coll_t<elem_t, args...> c)
				{
					{c.begin()};
					{c.end()};
					iterator_req<decltype(c.begin())>;
				};

				/**
				 * @brief defines additional requirement of size method for given collection
				 * 
				 * @tparam coll_t type to check
				 */
				template<template<typename T, typename... argv> typename coll_t, typename elem_t,
							typename... args>
				concept iterable_size_req
					 = iterable_req<coll_t, elem_t, args...>&& requires(coll_t<elem_t, args...> c)
				{
					{
						c.size()
					}
					->std::convertible_to<size_t>;
				};

				/**
				 * @brief defines additional requirement of empty method for given collection
				 * 
				 * @tparam coll_t type to check
				 */
				template<template<typename T, typename... argv> typename coll_t, typename elem_t,
							typename... args>
				concept iterable_size_empty_req
					 = iterable_size_req<coll_t, elem_t, args...>&& requires(coll_t<elem_t, args...> c)
				{
					{
						c.empty()
					}
					->std::same_as<bool>;
				};

				namespace detail
				{
					/**
					 * @brief provides universal processing interface for iterable types
					 * 
					 * @tparam coll_t collection type
					 * @tparam elem_t element type in collection
					 * @tparam param_t put here `const coll_t<elem_t>&` or `coll_t<elem_t>&`
					 * @tparam args additional parameters for collection (ex. comparator)
					 */
					template<typename processing_impl,
								template<typename _, typename... __> typename coll_t, typename elem_t,
								typename param_t, typename... args>
					struct processing_base
					{
						template<typename stream_t>
						requires iterable_req<coll_t, elem_t, args...> processing_base(stream_t& os,
																											param_t c)
						{
							processing_impl(os, c, std::distance(c.begin(), c.end()));
						}

						template<typename stream_t>
						requires iterable_size_req<coll_t, elem_t, args...> processing_base(stream_t& os,
																												  param_t c)
						{
							processing_impl(os, c, c.size());
						}

						template<typename stream_t>
						requires iterable_size_empty_req<coll_t, elem_t, args...> processing_base(
							 stream_t& os, param_t c)
						{
							// some of collections has to iterate over themselfs to get size, so if it's possible to avoid it, why not?u
							processing_impl(os, c, (c.empty() ? 0 : c.size()));
						}
					};

					/**
					 * @brief helper struct to put elements into array
					 * 
					 * @tparam elem_t type of element
					 * @tparam size size of array
					 */
					template<typename elem_t, size_t size> struct array_putter
					{
						std::array<elem_t, size>& arr;
						size_t idx = 0;

						bool operator()(const elem_t& e)
						{
							dassert{idx++ < size, "out of range"_u8};
							arr[idx] = e;
							return true;
						}
					};

					/**
					 * @brief emplace data to collection
					 * 
					 * @tparam coll_t type of collection
					 * @tparam elem_t type of element
					 * @tparam argv additional template parameters for collections
					 */
					template<template<typename T, typename... argv> typename coll_t, typename elem_t,
								typename... argv>
					struct emplacer
					{
						coll_t<elem_t, argv...>& coll;

						bool operator()(const elem_t& e)
						{
							coll.emplace(e);
							return true;
						}
					};
				}	 // namespace detail

				using detail::array_putter;
				using detail::emplacer;

				/**
				 * @brief serializes any collection
				 * 
				 * @tparam coll_t collection type to serialize
				 * @tparam elem_t element type of collection
				 */
				template<template<typename _, typename... args> typename coll_t, typename elem_t,
							typename... args>
				struct serial_impl
				{
					template<typename stream_t>
					serial_impl(stream_t& os, const coll_t<elem_t, args...>& c, const size_t size)
					{
						using patterns::serial::delimiter;
						os << size << delimiter;
						if(size == 0) return;
						for(const auto& e: c) os << e << delimiter;
					}
				};

				/**
				 * @brief deserializes any collection
				 * 
				 * @tparam coll_t collection type to deserialize
				 * @tparam elem_t element type of collection
				 * @tparam coll_putter this class will be used to insert elem_t to coll_t
				 */
				template<typename coll_putter, template<typename _, typename... args> typename coll_t,
							typename elem_t, typename... args>
				struct deserial_impl
				{
					template<typename stream_t>
					deserial_impl(stream_t& is, coll_t<elem_t, args...>& c, const size_t)
					{
						size_t size;
						is >> size;
						drop_delimiter(is);
						if(size == 0) return;
						coll_putter cp{c};
						for(size_t i = 0; i < size; ++i)
						{
							elem_t e;
							is >> e;
							drop_delimiter(is);
							if(!cp(e)) break;
						}
					}
				};

				/**
				 * @brief pretty prints any collection
				 * 
				 * @tparam coll_t collection type to serialize
				 * @tparam elem_t element type of collection
				 */
				template<template<typename _, typename... args> typename coll_t, typename elem_t,
							typename... args>
				struct pretty_print_impl
				{
					template<typename stream_t>
					pretty_print_impl(stream_t& os, const coll_t<elem_t, args...>& c, const size_t size)
					{
						os << '[';
						for(auto it = c.begin(); it != c.end(); it++)
							os << ","[it == c.begin()] << ' ' << pretty_print{*it};
						os << " ]";
					}
				};

				/**
				 * @brief general use serialization alias for collections
				 * 
				 * @tparam coll_t type of collection
				 * @tparam elem_t type of element
				 * @tparam args additional parameters for collection
				 */
				template<template<typename _, typename... args> typename coll_t, typename elem_t,
							typename... args>
				using serial = detail::processing_base<serial_impl<coll_t, elem_t, args...>, coll_t,
																	elem_t, const coll_t<elem_t, args...>&, args...>;

				/**
				 * @brief general use deserialization alias for collections
				 * 
				 * @tparam coll_putter class that implements adding following items to collection
				 * @tparam coll_t type of collection
				 * @tparam elem_t type of element
				 * @tparam args additional parameters for collection
				 */
				template<typename coll_putter, template<typename _, typename... args> typename coll_t,
							typename elem_t, typename... args>
				using deserial
					 = detail::processing_base<deserial_impl<coll_putter, coll_t, elem_t, args...>,
														coll_t, elem_t, coll_t<elem_t, args...>&, args...>;

				/**
				 * @brief general use pretty-printing alias for collections
				 * 
				 * @tparam coll_t type of collection
				 * @tparam elem_t type of element
				 * @tparam args additional parameters for collection
				 */
				template<template<typename _, typename... args> typename coll_t, typename elem_t,
							typename... args>
				using pretty_print
					 = detail::processing_base<pretty_print_impl<coll_t, elem_t, args...>, coll_t,
														elem_t, const coll_t<elem_t, args...>&, args...>;
			};	  // namespace collection

			/**
			 * @brief definition of serializing map
			 * 
			 * @tparam T any type
			 */
			template<typename Key, typename Value> struct map_serial
			{
				template<typename stream_type>
				map_serial(stream_type& os, const std::map<Key, Value>& data)
				{
					using patterns::serial::delimiter;
					os << data.size() << delimiter;
					for(const auto& pair: data)
						os << pair.first << delimiter << pair.second << delimiter;
				}
			};

			/**
			 * @brief definition of deserializing map
			 * 
			 * @tparam T any type
			 */
			template<typename Key, typename Value> struct map_deserial
			{
				template<typename stream_type> map_deserial(stream_type& is, std::map<Key, Value>& data)
				{
					size_t size;
					is >> size;
					drop_delimiter(is);
					if(size == 0) return;
					for(size_t i = 0; i < size; ++i)
					{
						Key key;
						is >> key;
						drop_delimiter(is);
						Value val;
						is >> val;
						drop_delimiter(is);
						data.emplace(key, std::move(val));
					}
				}
			};

			/**
			 * @brief definition of pretty serializing map
			 * 
			 * @tparam T any type
			 */
			template<typename Key, typename Value, typename key_pretty_printer>
			struct map_pretty_serial
			{
				template<typename stream_type>
				map_pretty_serial(stream_type& os, const std::map<Key, Value>& data)
				{
					os << '[';
					for(auto it = data.begin(); it != data.end(); it++)
						os << ","[it == data.begin()] << " ( " << key_pretty_printer{it->first} << " : "
							<< patterns::serial::pretty_print{it->second} << " )";
					os << " ]";
				}
			};

			namespace enums
			{

				/**
			 * @brief defines requirements for translation unit
			 * 
			 * @tparam T type to check
			 */
				template<typename T> concept array_provider_req = requires
				{
					typename T::enum_t;
					typename T::base_enum_t;
					{
						T::length + 1
					}
					->std::same_as<size_t>;
					std::same_as<std::remove_reference_t<decltype(T::translation[0])>, u16str>;
					std::convertible_to<std::numeric_limits<typename T::base_enum_t>,
											  typename T::enum_t>;
				};

				/**
			 * @brief universal way of stringinizing enums
			 * 
			 * @tparam array_provider which fullfills requirements of array_provider_req
			 */
				template<array_provider_req array_provider> struct enum_stringinizer
				{
					using enum_t										 = array_provider::enum_t;
					using base_enum_t									 = array_provider::base_enum_t;
					constexpr static size_t length				 = array_provider::length;
					inline static const u16str* enum_to_string = array_provider::translation;
					constexpr static base_enum_t not_found		 = std::numeric_limits<base_enum_t>::max();

					const enum_t x;

					/**
				 * @brief converts given enum to string
				 * 
				 * @param x input enum
				 * @return u16str stringinized version
				 */
					static u16str get(const enum_t x)
					{
						const size_t index = static_cast<size_t>(x);
						// dassert(index < length, "invalid enum_t"_u8);
						if(index >= length)
						{
							global_logger.warn() << "not found enum: " << index << logger::endl;
							return u"NOT FOUND";
						}
						return enum_stringinizer::enum_to_string[index];
					}

					/**
				 * @brief converts given string to enum
				 * 
				 * @param x input string
				 * @return enum_t converted string
				 * @return (enum_t)(std::numeric_limits<enum_t>::max()) if not found
				 */
					static enum_t get(u16str x)
					{
						std::for_each(x.begin(), x.end(), [](u16char_t& c) { c = std::toupper(c); });
						for(size_t i = 0; i < length; ++i)
							if(x == enum_to_string[i]) return static_cast<enum_t>(i);
						global_logger.warn() << "invalid string: " << x << logger::endl;

						return static_cast<enum_t>(not_found);
					}

					/**
				 * @brief operator for eazier stringinization
				 * 
				 * @tparam stream_t any stream
				 * @param os ref to stream
				 * @param x self
				 * @return stream_t& given string
				 */
					template<typename stream_t>
					inline friend stream_t& operator<<(stream_t& os, const enum_stringinizer& x)
					{
						return os << get_conversion_engine().to_bytes(get(x.x));
					}
				};

				/**
				 * @brief universal wrapper for enumerators
				 * 
				 * @tparam array_provider provides string translation
				 */
				template<array_provider_req array_provider>
				struct detail_enum_t : public serial_helper_t
				{
					using enum_type = array_provider::enum_t;
					dser<&detail_enum_t::_, enum_type> data;

					inline friend bool operator<(const detail_enum_t& e1, const detail_enum_t& e2)
					{
						return e1.data() < e2.data();
					}

					struct custom_serialize
					{
						template<typename stream_t> custom_serialize(stream_t& os, const enum_type& e)
						{
							os << static_cast<int>(e) << delimiter;
						}
					};

					struct custom_deserialize
					{
						template<typename stream_t> custom_deserialize(stream_t& is, enum_type& e)
						{
							is >> e;
							drop_delimiter(is);
							// os << static_cast<int>(e) << delimiter;
						}
					};

					struct custom_pretty_print
					{
						template<typename stream_t> custom_pretty_print(stream_t& os, const enum_type& e)
						{
							string::pretty_print{os, enum_stringinizer<array_provider>::get(e)};
						}
					};
				};
				template<array_provider_req array_provider>
				using enum_t = cser<&detail_enum_t<array_provider>::data>;
			}	 // namespace enums

			/**
			 * @brief alias for serializing shared vector
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in shared vector
			 */
			template<auto X, typename Key, typename Value>
			using map_ser								= ser<X, std::map<Key, Value>>;
			template<typename T> using shared_t = shared::single_member_shared_struct_helper<T>;
		}	 // namespace processing_details
	}		 // namespace objects
}	 // namespace core