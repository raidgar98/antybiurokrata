#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <iomanip>
#include <array>
#include <set>
#include <map>

namespace core
{
	namespace objects
	{
		namespace serial_definitions
		{
			using namespace patterns::serial;
			using patterns::serial::serial_helper_t;

			template<typename T> struct shared_compare
			{
				using sT = std::shared_ptr<T>;
				bool operator()(const sT& s1, const sT& s2) const
				{
					dassert(s1 && s2, "one of pointers is nullptr!"_u8);
					return *s1 < *s2;
				}
			};

			struct u16str_serial
			{
				template<typename stream_type> u16str_serial(stream_type& os, const u16str_v& view)
				{
					using patterns::serial::delimiter;
					os << view.size() << delimiter;
					for(const auto c: view) os << static_cast<int>(c) << delimiter;
				}
			};

			struct u16str_deserial
			{
				template<typename stream_type> u16str_deserial(stream_type& is, u16str& out)
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

			struct u16str_pretty_serial
			{
				template<typename stream_type>
				u16str_pretty_serial(stream_type& os, const u16str_v& view)
				{
					os << get_conversion_engine().to_bytes(view.data());
				}
			};

			/**
			 * @brief definition of any array serialization
			 * 
			 * @tparam T any type
			 * @tparam N any size
			 */
			template<typename T, size_t N> struct array_serial
			{
				template<typename stream_type>
				array_serial(stream_type& os, const std::array<T, N>& data)
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
			template<typename T, size_t N> struct array_deserial
			{
				template<typename stream_type> array_deserial(stream_type& is, std::array<T, N>& data)
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
			template<typename T, size_t N> struct array_pretty_serial
			{
				template<typename stream_type>
				array_pretty_serial(stream_type& os, const std::array<T, N>& data)
				{
					using patterns::serial::delimiter;
					os << '[';
					for(size_t i = 0; i < N; i++) os << ","[i == 0] << ' ' << data[i];
					os << " ]";
				}
			};

			/** @brief specialisation of pretty print designed for orcid */
			template<> struct array_pretty_serial<uint16_t, 4>
			{
				template<typename stream_type>
				array_pretty_serial(stream_type& os, const std::array<uint16_t, 4>& data)
				{
					for(size_t i = 0; i < 4; i++)
						os << "-"[i == 0] << std::setw(4) << std::setfill('0') << std::to_string(data[i]);
				}
			};

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

			/**
			 * @brief definition of deserializing shared set
			 * 
			 * @tparam T any type
			 */
			template<typename T> struct shared_set_deserial
			{
				template<typename stream_type>
				shared_set_deserial(stream_type& is,
										  std::set<std::shared_ptr<T>, shared_compare<T>>& data)
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
										std::set<std::shared_ptr<T>, shared_compare<T>>& data)
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
			using sset_ser = ser<X, std::set<std::shared_ptr<T>, shared_compare<T>>>;
			template<typename T>
			using shared_set_pretty_serial
				 = shared_collection_pretty_serial<std::set<std::shared_ptr<T>, shared_compare<T>>>;

			/**
			 * @brief alias for serializing shared vector
			 * 
			 * @tparam X reference to previous member
			 * @tparam T type in shared vector
			 */
			template<auto X, typename Key, typename Value>
			using map_ser = ser<X, std::map<Key, Value>>;

		}	 // namespace serial_definitions
	}		 // namespace objects
}	 // namespace core