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
						using patterns::serial::delimiter;
						int is_null;
						is >> is_null;
						is.ignore(1, delimiter);

						if(is_null != 0)
						{
							ptr = std::make_shared<T>();
							is >> *ptr;
							is.ignore(1, delimiter);
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
						if(ptr) os << pretty_print{*ptr};
						else
							os << null_value_string;
					}
				};
			}	 // namespace shared

			/** @brief contatins custom processing options for u16str */
			namespace u16str
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
						using patterns::serial::delimiter;
						size_t size;
						is >> size;
						if(size == 0) return;
						else
							out.reserve(size);
						for(size_t i = 0; i < size; ++i)
						{
							int c;
							is >> c;
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
			}	 // namespace u16str

			namespace collection
			{
				/**
				 * @brief defines minimum requirements for iterator
				 * 
				 * @tparam iterator_t type to check
				 */
				template<typename iterator_t> concept iterator_req = requires(itrator_t it)
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
				template<template<typename T> typename coll_t>
				concept iterable_req = requires(coll_t<int> c)
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
				template<template<typename T> typename coll_t>
				concept iterable_size_req = iterable_req<coll_t>&& requires(coll_t<int> c)
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
				template<template<typename T> typename coll_t>
				concept iterable_size_empty_req = iterable_size_req<coll_t>&& requires(coll_t<int> c)
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
				 * @tparam coll_t 
				 * @tparam elem_t 
				 */
					template<template<typename _> typename coll_t, typename elem_t,
								typename param_t = const coll_t<elem_t>&>
					struct processing_base
					{
						// [S]pecialized collection type
						using scoll_t = coll_t<elem_t>;

						template<typename stream_t>
						requires iterable_req<coll_t> processing_base(stream_t& os, param_t c)
						{
							processing_impl(os, c, std::distance(c.begin(), c.end()));
						}

						template<typename stream_t>
						requires iterable_size_req<coll_t> processing_base(stream_t& os, param_t c)
						{
							processing_impl(os, c, c.size());
						}

						template<typename stream_t>
						requires iterable_size_req<coll_t> processing_base(stream_t& os, param_t c)
						{
							// some of collections has to iterate over themselfs to get size, so if it's possible to avoid it, why not?u
							processing_impl(os, c, (c.empty() ? 0 : c.size()));
						}

					 protected:
						template<typename stream_t>
						virtual void processing_impl(stream_t& os, param_t c,
															  const size_t size) const = 0;
					};
				}	 // namespace detail

				/**
				 * @brief serializes any collection
				 * 
				 * @tparam coll_t collection type to serialize
				 * @tparam elem_t element type of collection
				 */
				template<template<typename _> typename coll_t, typename elem_t>
				struct serial : public detail::processing_base<coll_t, elem_t>
				{
					using detail::processing_base<coll_t, elem_t>::processing_base;
					using scoll_t = typename detail::processing_base<coll_t, elem_t>::scoll_t;

				 protected:
					virtual void processing_impl(stream_t& os, const scoll_t& c,
														  const size_t size) const override
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
				template<template<typename _> typename coll_t, typename elem_t, typename coll_putter>
				struct deserial : public detail::processing_base<coll_t, elem_t, coll_t<elem_t>&>
				{
					using detail::processing_base<coll_t, elem_t, coll_t<elem_t>&>::processing_base;
					using scoll_t = typename detail::processing_base<coll_t, elem_t>::scoll_t;

				 protected:
					virtual void processing_impl(stream_t& os, scoll_t& c,
														  const size_t size) const override
					{
						using patterns::serial::delimiter;
						size_t size;
						os >> size;
						os.ignore(1, delimiter);
						if(size == 0) return;
						for(size_t i = 0; i < size; ++i)
						{
							elem_t e;
							os >> e;
							os.ignore(1, delimiter);
							coll_putter{c, e};
						}
					}
				};

				/**
				 * @brief pretty prints any collection
				 * 
				 * @tparam coll_t collection type to serialize
				 * @tparam elem_t element type of collection
				 */
				template<template<typename _> typename coll_t, typename elem_t>
				struct pretty_print : public detail::processing_base<coll_t, elem_t>
				{
					using detail::processing_base<coll_t, elem_t>::processing_base;
					using scoll_t = typename detail::processing_base<coll_t, elem_t>::scoll_t;

				 protected:
					virtual void processing_impl(stream_t& os, const scoll_t& c,
														  const size_t size) const override
					{
						using patterns::serial::delimiter;
						os << '[';
						for( auto it = c.begin(); it != c.end(); it++ ) os << ","[it == c.begin()] << ' ' << pretty_print{ *it };
						os << " ]";
					}
				};
			};	  // namespace collection

			/** @brief contatins custom processing options for std::array */
			namespace array
			{
				/**
				 * @brief definition of any array serialization
				 * 
				 * @tparam T any type
				 * @tparam N any size
				 */
				template<typename T, size_t N> struct serial
				{
					template<typename stream_type> serial(stream_type& os, const std::array<T, N>& data)
					{
						using patterns::serial::delimiter;
						for(size_t i = 0; i < N; i++) os << data[i] << delimiter;
					}
				};

				/**
				 * @brief definition of any array deserialization
				 * 
				 * @tparam T any type
				 * @tparam N any size
				 */
				template<typename T, size_t N> struct deserial
				{
					template<typename stream_type> deserial(stream_type& is, std::array<T, N>& data)
					{
						using patterns::serial::delimiter;
						for(size_t i = 0; i < N; i++)
						{
							is >> data[i];
							is.ignore(1, delimiter);
						}
					}
				};

				/**
			 * @brief definition of any array pretty print
			 * 
			 * @tparam T any type
			 * @tparam N any size
			 */
				template<typename T, size_t N> struct pretty_print
				{
					template<typename stream_type>
					pretty_print(stream_type& os, const std::array<T, N>& data)
					{
						using patterns::serial::delimiter;

						os << '[';
						for(size_t i = 0; i < N; i++) os << ","[i == 0] << ' ' << pretty_print{data[i]};
						os << " ]";
					}
				};

				/** @brief specialisation of pretty print designed for orcid */
				struct orcid_pretty_print
				{
					template<typename stream_type>
					orcid_pretty_print(stream_type& os, const std::array<uint16_t, 4>& data)
					{
						for(size_t i = 0; i < 4; i++)
							os << "-"[i == 0] << std::setw(4) << std::setfill('0')
								<< std::to_string(data[i]);
					}
				};
			}	 // namespace array


			namespace vector
			{
				/**
			 * @brief definition of serializing shared vector
			 * 
			 * @tparam T any type
			 */
				template<typename T> struct shared_vector_serial
				{
					template<typename stream_type>
					shared_vector_serial(stream_type& os, const std::vector<std::shared_ptr<T>>& data)
					{
						using patterns::serial::delimiter;
						os << data.size() << delimiter;
						for(auto x: data)
						{
							if(x.get()) os << 1 << delimiter << *x << delimiter;
							else
								os << 0 << delimiter;
						}
					}
				};

				/**
				 * @brief definition of deserializing shared vector
				 * 
				 * @tparam T any type
				 */
				template<typename T> struct shared_vector_deserial
				{
					template<typename stream_type>
					shared_vector_deserial(stream_type& is, std::vector<std::shared_ptr<T>>& data)
					{
						using patterns::serial::delimiter;
						size_t size;
						is >> size;
						is.ignore(1, delimiter);
						if(size == 0) return;
						data.reserve(size);
						for(size_t i = 0; i < size; i++)
						{
							int proto_bool;
							is >> proto_bool;
							is.ignore(1, delimiter);
							if(proto_bool)
							{
								std::shared_ptr<T> ptr{new T{}};
								is >> *ptr;
								data.push_back(ptr);
							}
							is.ignore(1, delimiter);
						}
					}
				};
			}	 // namespace vector

			/**
			 * @brief definition of deserializing shared set
			 * 
			 * @tparam T any type
			 */
			template<typename T> struct shared_set_deserial
			{
				template<typename stream_type>
				shared_set_deserial(stream_type& is,
										  std::set<std::shared_ptr<T>, shared::compare<T>>& data)
				{
					using patterns::serial::delimiter;
					size_t size;
					is >> size;
					is.ignore(1, delimiter);
					if(size == 0) return;
					for(size_t i = 0; i < size; i++)
					{
						int proto_bool;
						is >> proto_bool;
						is.ignore(1, delimiter);
						if(proto_bool)
						{
							std::shared_ptr<T> ptr{new T{}};
							is >> *ptr;
							data.emplace(ptr);
						}
						is.ignore(1, delimiter);
					}
				}
			};

			/**
			 * @brief definition of serializing shared vector
			 * 
			 * @tparam T any type
			*/
			template<typename T> struct shared_set_serial
			{
				template<typename stream_type>
				shared_set_serial(stream_type& is,
										std::set<std::shared_ptr<T>, shared::compare<T>>& data)
				{
					using patterns::serial::delimiter;
					is << data.size() << delimiter;
					for(auto x: data)
					{
						if(x.get()) is << 1 << delimiter << *x;
						else
							is << 0;

						is << delimiter;
					}
				}
			};

			/**
			 * @brief definition of pretty printing shared collection
			 * 
			 * @tparam Coll any collection
			 */
			template<typename Coll> struct shared_collection_pretty_serial
			{
				template<typename stream_type>
				shared_collection_pretty_serial(stream_type& os, const Coll& data)
				{
					os << '[';
					// for(size_t i = 0; i < data.size(); ++i)
					for(auto it = data.begin(); it != data.end(); ++it)
						if(it->get())
							os << ","[it == data.begin()] << ' ' << patterns::serial::pretty_print{*(*it)};
					os << " ]";
				}
			};

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
					using patterns::serial::delimiter;
					size_t size;
					is >> size;
					is.ignore(1, delimiter);
					if(size == 0) return;
					for(size_t i = 0; i < size; ++i)
					{
						Key key;
						is >> key;
						is.ignore(1, delimiter);
						Value val;
						is >> val;
						is.ignore(1, delimiter);
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

			/**
			 * @brief universal printer for enums
			 * 
			 * @tparam enum_t anny enum type
			 * @tparam cast_type type to cast
			 */
			template<typename enum_t, typename cast_type = int> struct enum_printer
			{
				const enum_t& x;

				template<typename stream_t>
				inline friend stream_t& operator<<(stream_t& os, const enum_printer& obj)
				{
					return os << cast_type(obj.x);
				}
			};

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
				std::convertible_to<std::numeric_limits<typename T::base_enum_t>, typename T::enum_t>;
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
			 * @brief alias for serializing any array
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in array
			 * @tparam N array size
			 */
			template<auto X, typename T, size_t N> using array_ser = ser<X, std::array<T, N>>;

			/**
			 * @brief alias for serializing shared vector
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in shared vector
			 */
			template<auto X, typename T> using svec_ser = ser<X, std::vector<std::shared_ptr<T>>>;
			template<typename T>
			using shared_vector_pretty_serial
				 = shared_collection_pretty_serial<std::vector<std::shared_ptr<T>>>;

			/**
			 * @brief alias for serializing shared set
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in shared set
			 */
			template<auto X, typename T>
			using sset_ser = ser<X, std::set<std::shared_ptr<T>, shared::compare<T>>>;
			template<typename T>
			using shared_set_pretty_serial
				 = shared_collection_pretty_serial<std::set<std::shared_ptr<T>, shared::compare<T>>>;

			/**
			 * @brief alias for serializing shared vector
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in shared vector
			 */
			template<auto X, typename Key, typename Value>
			using map_ser = ser<X, std::map<Key, Value>>;

		}	 // namespace processing_details
	}		 // namespace objects
}	 // namespace core