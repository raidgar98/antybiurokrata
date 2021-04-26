/**
 * @file seiralizer.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief 
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 */

/** 
 * @example "serialization ~ usage"
 * definig serializable class:
 * ```
 * struct _Example : public serial_helper_t
 * {
 * 	ser<&_Example::_, int> a{1};
 * 	ser<&_Example::a, int> b{2};
 * 	ser<&_Example::b, int> c{3};
 * };
 * using Example = cser<&_Example::c>;
 * ```
 * * * * * * * * * * *
 * 
 * usage in code:
 * ```
 * int main()
 * {
 * 	Example ex{10, 20, 30};
 * 	std::cout << ex; // prints: 10 20 30
 * 	std::cout << pretty_print{ex}; // prints: _Example[ 10, 20, 30 ]
 * 	
 * 	std::ofstream ofile("myfile.bin");
 * 	ofile << ex; // in file: 10\020\030\0
 * 	ofile.close()
 * 
 * 	ex.a = 0;
 * 	ex.b = 0;
 * 	ex.c = 0;
 * 
 * 	std::cout << ex; // prints: 0 0 0
 * 	std::cout << pretty_print{ex}; // prints: _Example[ 0, 0, 0 ]
 * 
 * 	std::ifstream ifile("myfile.bin");
 * 	ifile >> ex;
 * 	ifile.close();
 * 
 * 	std::cout << ex; // prints: 10 20 30
 * 	std::cout << pretty_print{ex}; // prints: _Example[ 10, 20, 30 ]
 * 
 * 	return 0; 
 * }
 * ```*/
#pragma once

#include <tuple>
#include <iostream>
#include <boost/type_index.hpp>

namespace patterns
{
	namespace serial
	{
		/**
		 * @brief serialization terminator
		 */
		struct ___null_t
		{
			void serialize(std::ostream &, const void *) const {}
			bool serialize_coma_separated(std::ostream &, const void *) const { return false; }
			void deserialize(std::istream &, const void *) const {}
		};

		/**
		 * @brief serialization helper. all it does is to append null_t as member (which mean end of serialization)
		*/
		struct serial_helper_t
		{
			___null_t _{};
		};

		/** @brief this is put to stream as separator for members */
		constexpr char delimiter{' '};

		/** @brief This class is used by ser to deserialize next members */
		struct get_from_stream
		{
			/**
			 * @brief specialize this constructor for more complex types if required
			 * 
			 * @tparam stream_t any stream
			 * @tparam Any any value_type
			 * @param is reference to stream
			 * @param any reference to object
			 */
			template<typename stream_t, typename Any>
			get_from_stream( stream_t& is, Any& any )
			{
				is >> any;
			}
		};

		/** @brief This class is used by ser to serialize class members */
		struct put_to_stream
		{
			/**
			 * @brief Construct specialize this constructor for more complex types if required
			 * 
			 * @tparam stream_t any stream
			 * @tparam Any any value_type
			 * @param os reference to stream
			 * @param any const reference to stream
			 */
			template<typename stream_t, typename Any>
			put_to_stream( stream_t& os, const Any& any )
			{
				os << any;
			}
		};

		template<typename T>
		concept serializable_req = requires(T x)
		{
			{ put_to_stream( std::cout, x ) };
			{ get_from_stream( std::cin, x ) };
		};
			// std::is_constructible_v<get_from_stream, std::istream, T>
		// and
			// std::is_constructible_v<put_to_stream, std::ostream, T>
		// ;

		/**
		 * @brief SE(rialization) R(ecursive) wrapper for struct/class members 
		 * 
		 * @tparam value reference to previous struct/class member (Ex.: &MyClass::member_0)
		 * @tparam T type of current member
		 */
		template <auto value, typename T, class serial_methode, class deserial_methode>
		struct ser {};

		/** @brief d[efault] ser */
		template<auto value, typename T>
		using dser = ser<value, T, put_to_stream, get_from_stream>;

		/**
		 * @brief ser implementation by specialization
		 * 
		 * @tparam Class type of owner class
		 * @tparam Result type of previous member
		 * @tparam Class::*value reference to member in class
		 * @tparam T type of current member
		*/
		template <typename Class, typename Result, Result Class::*value, serializable_req T, typename serial_methode, typename deserial_methode>
		struct ser<value, T, serial_methode, deserial_methode>
		{

			using value_type = T;
			/** wrapped value */
			value_type val;

			/**
			 * @brief forward constructor
			 * 
			 * @tparam U any types required by wrapped type T
			 * @param u any values of (any) types U required by wrapped type T
			 */
			template <typename... U>
			ser(U &&...u) : val{std::forward<U>(u)...} {}

			/**
			 * @brief forwarding move constructor for T type
			 * 
			 * @param v currently moving object
			 */
			ser(T &&v) : val{std::move(v)} {};

			/**
			 * @brief forwarding copy constructor for T type
			 * 
			 * @param v currently copying object
			 */
			ser(const T &v) : val{v} {};

			/**
			 * @brief forwards move assignment operator
			 * 
			 * @tparam U any other type
			 * @param v any value
			 * @return ser& return self
			*/
			template<typename U>
			ser& operator=(U&& v) { val = std::move(v); return *this; }

			/**
			 * @brief forwards copy assignment operator
			 * 
			 * @tparam U any other type
			 * @param v any value
			 * @return ser& return self
			*/
			template<typename U>
			ser& operator=(const U& v) { val = v; return *this; }

			/**
			 * @brief thanks to this operator, this wrapper is also getter
			 * 
			 * @return value_type& modifable reference to wrapped value
			 */
			value_type& operator()(void) { return val; }

			/**
			 * @brief same as previous, for compilator to decide which use when
			 * 
			 * @return const value_type& reference to wrapped value
			*/
			const value_type& operator()(void) const { return val; }

			/**
			 * @brief thanks to this operator, this wrapper is also setter
			 * 
			 * @tparam U Any value convertible to value_type
			 * @param u data to set
			 */
			template<typename U>
			void operator()(const U& u) { val = u; }

			/**
			 * @brief copying constructor for others members with same type
			 * 
			 * @tparam U any other member from same/other class
			 * @tparam _T any other (or same) type convertible to T
			 * @param v any wrapper
			 */
			template <auto ... U>
			explicit ser(const ser<U...> &v) : val{v.val} {};

			/**
			 * @brief serializes current member and forward serialization to next member 
			 * 
			 * @tparam stream_t any output stream type (Ex. std::ofstream, std::cout)
			 * @param os reference to stream
			 * @param that pointer to owner class (this)
			 */
			template <typename stream_t>
			void serialize(stream_t &os, const Class *that) const
			{
				(that->*value).serialize(os, that);
				serial_methode( os, this->val );
				os << delimiter;
			}

			/**
			 * @brief serializes current member and forward serialization to next member, but members are coma separated (pretty)
			 * 
			 * @tparam stream_t any output stream type (Ex. std::ofstream, std::cout)
			 * @param os reference to stream
			 * @param that pointer to owner class (this)
			 * @return true if coma should be added begore current member
			 * @return false if coma shouldn't be placed
			 */
			template <typename stream_t>
			bool serialize_coma_separated(stream_t &os, const Class *that) const
			{
				if ((that->*value).serialize_coma_separated(os, that))
					os << ", ";

				serial_methode(os, this->val);
				return true;
			}

			/**
			 * @brief reads from given stream, deserializing wrapped type and forward it to next member
			 * 
			 * @tparam stream_t any input stream type (Ex. std::ifstream, std::cin)
			 * @param os reference to stream
			 * @param that pointer to owner class (this)
			 */
			template <typename stream_t>
			void deserialize(stream_t &is, Class *that)
			{
				(that->*value).deserialize(is, that);

				deserial_methode(is, this->val);
				is.ignore(1, delimiter);
			}

			/**
			 * @brief equal operator for lazy people
			 * 
			 * @param s1 left part
			 * @param s2 right part
			 * @return true if same
			 * @return false if not
			 */
			friend inline bool operator==(const ser& s1, const ser& s2) { return s1() == s2(); }
		};

		/**
		 * @brief C(lass) SE(rialization) R(ecursive); use this on class, that needs to be serialized
		 * 
		 * @tparam LastItemRef put here reference to last serialized member in class (Ex. &MyClass::member_n)
		 */
		template <auto LastItemRef>
		struct cser
		{
		};

		/**
		 * @brief ser implementation by specialization
		 * 
		 * @tparam Class type of serialized class
		 * @tparam Result type of last member
		 * @tparam Class::*value reference to last member in class
		*/
		template <typename Class, typename Result, Result Class::*last>
		struct cser<last>
		{
			using class_t = Class;

			/** wrapped value */
			class_t val{};

			/**
			 * @brief forwards all constructors to wrapped class type and adds ___null_t if required
			 * 
			 * @tparam U any types
			 * @param u any values of types U
			*/
			template <typename... U>
			// requires( std::is_constructible_v<class_t, ___null_t, U...> )
			cser(U &&...u) : val{___null_t{}, std::forward<U>(u)...} {}

			/**
			 * @brief forwards all constructors to wrapped class type
			 * 
			 * @tparam U any types
			 * @param u any values of types U
			*/
			template <typename... U>
			requires( std::is_constructible_v<class_t, U...> )
			cser(U &&...u) : val{std::forward<U>(u)...} {}

			/**
			 * @brief serializes recursively class
			 * 
			 * @tparam stream_t any output stream type (Ex. std::ofstream, std::cout)
			 * @param os reference to stream
			 */
			template <typename stream_t>
			void serialize(stream_t &os) const
			{
				(val.*last).serialize(os, &val);
			}

			/**
			 * @brief deserializes recursively class
			 * 
			 * @tparam stream_t any input stream type (Ex. std::ifstream, std::cin)
			 * @param is reference to stream
			 */
			template <typename stream_t>
			void deserialize(stream_t &is)
			{
				(val.*last).deserialize(is, &val);
			}

			/**
			 * @brief serializes recursively class, but members are coma separated (pretty)
			 * 
			 * @tparam stream_t any output stream type (Ex. std::ofstream, std::cout)
			 * @param os reference to stream
			*/
			template <typename stream_t>
			void serialize_coma_separated(stream_t &os) const
			{
				(val.*last).serialize_coma_separated(os, &val);
			}

			/**
			 * @brief thanks to this operator, this wrapper is also getter
			 * 
			 * @return class_t& modifable reference to wrapped value
			 */
			class_t& operator()(void) { return val; }

			/**
			 * @brief same as previous, for compilator to decide which use when
			 * 
			 * @return const class_t& reference to wrapped value
			*/
			const class_t& operator()(void) const { return val; }

			/**
			 * @brief thanks to this operator, this wrapper is also setter
			 * 
			 * @tparam U Any value convertible to value_type
			 * @param u data to set
			 */
			template<typename U>
			void operator()(const U& u) { val = u; }

			/**
			 * @brief equal operator for lazy peoples
			 * 
			 * @param c1 left operand
			 * @param c2 right operand
			 * @return true if both are same
			 * @return false if diffrent
			 */
			inline friend bool operator==(const cser& c1, const cser& c2) { return c1() == c2(); }
		};

		/**
		 * @brief helper functor to easly print wrapped class
		 * 
		 * @tparam T any cser
		*/
		template <auto T>
		struct pretty_print
		{
			using wrap_t = typename serial::cser<T>;
			const wrap_t &ref;
			explicit pretty_print(const wrap_t &obj) : ref{obj} {}
			inline friend std::ostream &operator<<(std::ostream &os, const pretty_print<T> &obj)
			{
				os << boost::typeindex::type_id<typename wrap_t::class_t>() << "[ ";
				obj.ref.serialize_coma_separated(os);
				return os << " ]";
			}
		};

		/**
		 * @brief start point of serialization for 
		 * 
		 * @tparam T param of cser template
		 * @tparam stream_t any output stream type (Ex. std::ofstream, std::cout)
		 * @param os reference to stream
		 * @param obj any cser
		 * @return stream_t& returns given stream
		*/
		template <auto T, typename stream_t = std::ostream>
		inline stream_t& operator<<(stream_t &os, const typename serial::cser<T> &obj)
		{
			obj.serialize(os);
			return os;
		}

		/**
		 * @brief start point of deserialization for 
		 * 
		 * @tparam T param of cser template
		 * @tparam stream_t any intput stream type (Ex. std::ifstream, std::cin)
		 * @param is reference to stream
		 * @param obj any cser
		 * @return stream_t& returns given stream
		*/
		template <auto T, typename stream_t = std::istream>
		inline stream_t& operator>>(stream_t &is, typename serial::cser<T> &obj)
		{
			obj.deserialize(is);
			return is;
		}
	};
};