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
#include <concepts>
#include <boost/type_index.hpp>

struct logger;
struct logger_piper;

namespace patterns
{
	/** @brief contains implementaion of serialization classes */
	namespace serial
	{
		/**
		 * @brief serialization terminator
		 */
		struct ___null_t
		{
			/** @brief terminator */
			bool accept(void*) { return false; }
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

		template<typename stream_t> inline void drop_delimiter(stream_t& is)
		{
			is.ignore(1, delimiter);
		}

		/**
		 * @brief C(lass) SE(rialization) R(ecursive); use this on class, that needs to be serialized
		 * 
		 * @tparam LastItemRef put here reference to last serialized member in class (Ex. &MyClass::member_n)
		 */
		template<auto LastItemRef> struct cser
		{
		};

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
			template<typename stream_t, typename Any> get_from_stream(stream_t& is, Any& any)
			{
				is >> any;
				drop_delimiter(is);
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
			template<typename stream_t, typename Any> put_to_stream(stream_t& os, const Any& any)
			{
				os << any << delimiter;
			}
		};

		template<template<auto X, typename... U> typename some_class, auto X, typename... U>
		concept serializable_class_type_req = requires(some_class<X, U...> x)
		{
			typename some_class<X, U...>::is_serializable_class;
			{x.val};
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
		template<auto value, typename T> struct ser
		{
		};

		/** @brief d[efault] ser */
		template<auto value, typename T> using dser = ser<value, T>;

		template<typename svisitor_t> concept cheese_visitor_req = requires(svisitor_t* x)
		{
			{
				x->visit(new int)
			}
			->std::same_as<bool>;
			{x->that};
			{x->last_result};
		};

		/**
		 * @brief ser implementation by specialization
		 * 
		 * @tparam class_t type of owner class
		 * @tparam class_member_t type of previous member
		 * @tparam class_t::*value reference to member in class
		 * @tparam T type of current member
		*/
		template<typename class_t, typename class_member_t, class_member_t class_t::*value,
					typename T>
		struct ser<value, T>
		{
			using is_serializable_class = std::true_type;
			using value_type				 = T;
			/** wrapped value */
			value_type val;

			/** @brief easier access to wrapped values */
			operator value_type&() { return val; }
			operator const value_type&() const { return val; }

			/**
			 * @brief forward constructor
			 * 
			 * @tparam U any types required by wrapped type T
			 * @param u any values of (any) types U required by wrapped type T
			 */
			template<typename... U> ser(U&&... u) : val{std::forward<U>(u)...} {}

			/**
			 * @brief forwarding move constructor for T type
			 * 
			 * @param v currently moving object
			 */
			ser(T&& v) : val{std::move(v)} {};

			/**
			 * @brief forwarding copy constructor for T type
			 * 
			 * @param v currently copying object
			 */
			ser(const T& v) : val{v} {};

			/**
			 * @brief specializes copy constructor for other wrappers
			 * 
			 * @tparam U any type
			 * @param v any ser
			 * @return ser& self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> ser(const serial<X, U...>& v) :
				 val{v.val}
			{
			}

			/**
			 * @brief specializes move constructor for other wrappers
			 * 
			 * @tparam U any type
			 * @param v any ser
			 * @return ser& self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> ser(serial<X, U...>&& v) :
				 val{std::move(v.val)}
			{
			}

			/**
			 * @brief forwards move assignment operator
			 * 
			 * @tparam U any other type
			 * @param v any value
			 * @return ser& return self
			*/
			template<typename U> ser& operator=(U&& v)
			{
				val = std::move(v);
				return *this;
			}

			/**
			 * @brief forwards copy assignment operator
			 * 
			 * @tparam U any other type
			 * @param v any value
			 * @return ser& return self
			*/
			template<typename U> ser& operator=(const U& v)
			{
				val = v;
				return *this;
			}

			/**
			 * @brief forwards copy assigment operator for any wrapper
			 * 
			 * @tparam U any type
			 * @param v any ser
			 * @return ser& self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> ser& operator=(
				 const serial<X, U...>& v)
			{
				val = v.val;
				return *this;
			}

			/**
			 * @brief forwards move assigment operator for any wrapper
			 * 
			 * @tparam U any type
			 * @param v any ser
			 * @return ser& self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> ser& operator=(serial<X, U...>&& v)
			{
				val = std::move(v.val);
				return *this;
			}

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
			template<typename U> void operator()(const U& u) { val = u; }

			/**
			 * @brief overload of previous for serializable objects
			 * 
			 * @tparam U Any value convertible to value_type
			 * @param u data to set
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> void operator()(
				 const serial<X, U...>& u)
			{
				val = u.val;
			}

			/**
			 * @brief overload of previous for serializable objects
			 * 
			 * @tparam U Any value convertible to value_type
			 * @param u data to set
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> void operator()(serial<X, U...>&& u)
			{
				val = std::move(u.val);
			}

			/**
			 * @brief acceptor for visitors that modifies content of this class
			 * 
			 * @tparam visitor_t any matched visitor type
			 * @param v visitor
			 * @return bool result from visitor
			 */
			template<cheese_visitor_req visitor_t> bool accept(visitor_t* v)
			{
				if(v == nullptr) throw std::invalid_argument{"visitor cannot be nullptr"};
				if(v->that == nullptr) throw std::invalid_argument{"that in visitor cannot be nullptr"};

				v->last_result = (reinterpret_cast<class_t*>(v->that)->*value).accept(v);
				return v->visit(this);
			}

			/**
			 * @brief acceptor for visitors that doesn't modifies content of this class
			 * 
			 * @tparam visitor_t any matched visitor type
			 * @param v visitor
			 * @return bool result from visitor
			 */
			template<cheese_visitor_req visitor_t> bool accept(visitor_t* v) const
			{
				if(v == nullptr) throw std::invalid_argument{"visitor cannot be nullptr"};
				if(v->that == nullptr) throw std::invalid_argument{"that in visitor cannot be nullptr"};

				v->last_result = (reinterpret_cast<class_t*>(v->that)->*value).accept(v);
				return v->visit(this);
			}
		};

		/**
		 * @brief ser implementation by specialization
		 * 
		 * @tparam class_t type of serialized class
		 * @tparam class_member_t type of last member
		 * @tparam class_t::*value reference to last member in class
		*/
		template<typename class_t, typename class_member_t, class_member_t class_t::*last>
		struct cser<last>
		{
			using value_t					 = class_t;
			using is_serializable_class = std::true_type;

			/** wrapped value */
			class_t val{};

			/** @brief easier access to wrapped values */
			operator class_t&() { return val; }
			operator const class_t&() const { return val; }

			/** @brief forwarding constructor */
			template<auto X> cser(cser<X>&& x) : val{std::move(x.val)} {}
			template<auto X> cser(const cser<X>& x) : val{x.val} {}

			/**
			 * @brief forwards all constructors to wrapped class type and adds ___null_t if required
			 * 
			 * @tparam U any types
			 * @param u any values of types U
			*/
			template<typename... U> cser(U&&... u) : val{___null_t{}, std::forward<U>(u)...} {}

			/**
			 * @brief forwards all constructors to wrapped class type
			 * 
			 * @tparam U any types
			 * @param u any values of types U
			*/
			template<typename... U>
			requires(std::is_constructible_v<class_t, U...>) cser(U&&... u) :
				 val{std::forward<U>(u)...}
			{
			}

			/** @brief forwarding assign operator */
			template<typename U> cser& operator=(U&& u)
			{
				val = std::move(u);
				return *this;
			}

			/**
			 * @brief move constructor for wrapper with other types
			 * 
			 * @tparam serial any serializable class
			 * @tparam X pointer to next element 
			 * @tparam U other template arguments
			 * @param u other serial
			 * @return self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> cser(serial<X, U...>&& u) :
				 val{std::move(u.val)}
			{
			}

			/**
			 * @brief copy constructor for wrapper with other types
			 * 
			 * @tparam serial any serializable class
			 * @tparam X pointer to next element 
			 * @tparam U other template arguments
			 * @param u other serial
			 * @return self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> cser(const serial<X, U...>& u) :
				 val{u.val}
			{
			}

			/**
			 * @brief move assign operators for wrapper with other types
			 * 
			 * @tparam serial any serializable class
			 * @tparam X pointer to next element 
			 * @tparam U other template arguments
			 * @param u other serial
			 * @return self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> cser operator=(serial<X, U...>&& u)
			{
				val = std::move(u.val);
				return *this;
			}

			/**
			 * @brief copy assign operators for wrapper with other types
			 * 
			 * @tparam serial any serializable class
			 * @tparam X pointer to next element 
			 * @tparam U other template arguments
			 * @param u other serial
			 * @return self
			 */
			template<template<auto X, typename... U> typename serial, auto X, typename... U>
			requires serializable_class_type_req<serial, X, U...> cser
			operator=(const serial<X, U...>& u)
			{
				val = u.val;
				return *this;
			}

			/**
			 * @brief acceptor for visitors that modifies content of this class
			 * 
			 * @tparam visitor_t any matched visitor type
			 * @param v visitor
			 * @return bool result from visited wrapped value
			 */
			template<cheese_visitor_req visitor_t> bool accept(visitor_t* v)
			{
				if(v == nullptr) throw std::invalid_argument{"visitor cannot be nullptr"};
				// if(v->that == nullptr) throw std::invalid_argument{"that in visitor cannot be nullptr"};

				auto* prev			= v->that;
				v->that				= &val;
				const bool result = (reinterpret_cast<class_t*>(v->that)->*last).accept(v);
				v->that				= prev;
				return result;
			}

			/**
			 * @brief acceptor for visitors that doesn't modifies content of this class
			 * 
			 * @tparam visitor_t any matched visitor type
			 * @param v visitor
			 * @return bool result from visited wrapped value
			 */
			template<cheese_visitor_req visitor_t> bool accept(visitor_t* v) const
			{
				if(v == nullptr) throw std::invalid_argument{"visitor cannot be nullptr"};
				// if(v->that == nullptr) throw std::invalid_argument{"that in visitor cannot be nullptr"};

				void* prev			= v->that;
				v->that				= reinterpret_cast<void*>(const_cast<class_t*>(&val));
				const bool result = (reinterpret_cast<class_t*>(v->that)->*last).accept(v);
				v->that				= prev;
				return result;
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
			template<typename... U> void operator()(U&&... u) { val(std::forward<U>(u)...); }

			inline friend bool operator<(const cser& c1, const cser& c2) { return c1.val < c2.val; }
			inline friend bool operator==(const cser& c1, const cser& c2) { return c1.val == c2.val; }
			inline friend bool operator!=(const cser& c1, const cser& c2) { return c1.val != c2.val; }
		};

		template<typename stream_t>
		concept allowed_stream_req
			 = std::is_same_v<
					 stream_t,
					 logger> || std::is_same_v<stream_t, logger_piper> || std::derived_from<stream_t, std::ostream>;


		template<typename stream_t, typename stream_action> struct stream_handler
		{
			stream_t& os;
			bool last_result{true};
			void* that = nullptr;

			template<typename Any> bool visit(Any* input) const
			{
				if(input) stream_action{os, input->val};
				return input != nullptr;
			}
		};

		struct pretty_printer_separator
		{
			constexpr static std::string_view formated_delimiter{", "};
			constexpr static std::string_view no_delimiter{" "};

			template<typename stream_t>
			pretty_printer_separator(stream_t& ss, const bool put_delimiter)
			{
				ss << (put_delimiter ? formated_delimiter : no_delimiter);
			}
		};

		template<typename stream_t, typename stream_action,
					typename last_result_action = pretty_printer_separator>
		struct stream_handler_with_last_result
		{
			stream_t& os;
			bool last_result{true};
			void* that = nullptr;

			template<typename Any> bool visit(Any* input) const
			{
				if(input)
				{
					last_result_action{os, this->last_result};
					stream_action{os, input->val};
				}
				return input != nullptr;
			}
		};

		/**
		 * @brief verifies whether class declared custom serialization methode
		 * 
		 * @tparam cser_t type to check
		 */
		template<typename cser_t> concept custom_serialization_req = requires
		{
			typename cser_t::custom_serialize;
		};

		/**
		 * @brief serialization marker in stream
		 * 
		 * @tparam X cser - Ex.: cser<&my_ser::member_2>;
		 */
		template<typename X = int> struct serialize
		{
			const X& x;

			/**
			 * @brief returns the stream handler for give stream
			 * 
			 * @tparam stream_t any stream type
			 * @param os stream reference
			 * @return constexpr auto handler
			 */
			template<typename stream_t> constexpr static auto get_stream_handler(stream_t& os)
			{
				return stream_handler<stream_t, put_to_stream>{os};
			}

			/** @brief override of above for types with custom serializator */
			template<typename stream_t>
			requires custom_serialization_req<typename X::value_t> constexpr static auto
			get_stream_handler(stream_t& os)
			{
				return stream_handler<stream_t, typename X::value_t::custom_serialize>{os};
			}
		};

		template<typename stream_t, auto X>
		inline stream_t& operator<<(stream_t& os, const serialize<serial::cser<X>>& obj)
		{
			auto vs = serialize<serial::cser<X>>::get_stream_handler(os);
			obj.x.accept(&vs);
			return os;
		}

		template<typename stream_t, auto X>
		inline stream_t& operator<<(stream_t& os, const serial::cser<X>& obj)
		{
			serialize xx{obj};
			return operator<<<stream_t, X>(os, xx);
		}

		/**
		 * @brief verifies whether class declared custom deserialization methode
		 * 
		 * @tparam cser_t type to check
		 */
		template<typename cser_t> concept custom_deserialization_req = requires
		{
			typename cser_t::custom_deserialize;
		};

		/**
		 * @brief deserialization marker in stream
		 * 
		 * @tparam X cser - Ex.: cser<&my_ser::member_2>;
		 */
		template<typename X = int> struct deserialize
		{
			X& x;

			/**
			 * @brief returns the stream handler for give stream
			 * 
			 * @tparam stream_t any stream type
			 * @param os stream reference
			 * @return constexpr auto handler
			 */
			template<typename stream_t> constexpr static auto get_stream_handler(stream_t& os)
			{
				return stream_handler<stream_t, get_from_stream>{os};
			}

			/** @brief override of above for types with custom serializator */
			template<typename stream_t>
			requires custom_deserialization_req<typename X::value_t> constexpr static auto
			get_stream_handler(stream_t& os)
			{
				return stream_handler<stream_t, typename X::value_t::custom_deserialize>{os};
			}
		};

		template<typename stream_t, auto X>
		inline stream_t& operator>>(stream_t& os, deserialize<serial::cser<X>>& obj)
		{
			auto vs = deserialize<serial::cser<X>>::get_stream_handler(os);
			obj.x.accept(&vs);
			return os;
		}

		template<typename stream_t, auto X>
		inline stream_t& operator>>(stream_t& os, serial::cser<X>& obj)
		{
			deserialize xx{obj};
			return operator>><stream_t, X>(os, xx);
		}


		/**
		 * @brief verifies whether class declared custom serialization methode
		 * 
		 * @tparam cser_t type to check
		 */
		template<typename cser_t> concept custom_pretty_print_req = requires
		{
			typename cser_t::custom_pretty_print;
		};

		/**
		 * @brief helper functor to easly print wrapped class
		 * 
		 * @tparam T any cser
		*/
		template<typename X, typename pretty_printer_default> struct pretty_print_impl
		{
			const X& x;


			/**
			 * @brief returns the stream handler for give stream
			 * 
			 * @tparam stream_t any stream type
			 * @param os stream reference
			 * @return constexpr auto handler
			 */
			template<typename stream_t> constexpr static auto get_stream_handler(stream_t& os)
			{
				return stream_handler_with_last_result<stream_t, pretty_printer_default>{os};
			}

			/** @brief override of above for types with custom serializator */
			template<typename stream_t>
			requires custom_pretty_print_req<typename X::value_t>
				 //  typename X::value_t::custom_pretty_print
				 constexpr static auto get_stream_handler(stream_t& os)
			{
				return stream_handler_with_last_result<stream_t,
																	typename X::value_t::custom_pretty_print>{os};
			}
		};

		/** @brief type marker to detect behaviour in stream */
		template<typename T> struct pretty_print
		{
			const T& x;
		};

		/** @brief This class is used by ser to serialize class members in pretty way */
		struct pretty_put_to_stream;

		/**
		 * @brief starts pretty-printing for given value
		 * 
		 * @tparam stream_t any stream type
		 * @tparam X should be deduced
		 * @param os reference to stream
		 * @param obj object to serialize
		 * @return stream_t& given stream
		 */
		template<typename stream_t, typename T>
		inline stream_t& operator<<(stream_t& os, const pretty_print<T>& obj)
		{
			os << obj.x;
			return os;
		}
		template<typename stream_t, auto X>
		inline stream_t& operator<<(stream_t& os, const pretty_print<serial::cser<X>>& obj)
		{
			const std::string type_name
				 = boost::typeindex::type_id<typename serial::cser<X>::value_t>().pretty_name();
			const size_t shevron			 = type_name.find('<');
			const size_t double_dot_pos = type_name.find_last_of(':', shevron);
			os << type_name.substr(double_dot_pos + 1) << "[";

			auto vs = pretty_print_impl<serial::cser<X>, pretty_put_to_stream>::get_stream_handler(os);
			obj.x.accept(&vs);

			os << " ]";
			return os;
		}

		struct pretty_put_to_stream
		{
			/**
			 * @brief specializes cnstructor for serializable types
			 * 
			 * @tparam stream_t any stream
			 * @tparam X any value_type
			 * @param os reference to stream
			 * @param any const reference to printed obj
			 */
			template<typename stream_t, auto X>
			pretty_put_to_stream(stream_t& os, const serial::cser<X>& any)
			{
				operator<<<stream_t, X>(os, pretty_print{any});
			}

			/**
			 * @brief pretty put tu stream in default way
			 * 
			 * @tparam stream_t any stream
			 * @tparam X any value_type
			 * @param os reference to stream
			 * @param any const reference to printed obj
			 */
			template<typename stream_t, typename T> pretty_put_to_stream(stream_t& os, const T& any)
			{
				os << any;
			}
		};

	};	  // namespace serial
};		  // namespace patterns
