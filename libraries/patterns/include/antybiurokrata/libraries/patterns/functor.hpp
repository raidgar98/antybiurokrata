/**
 * @file functor.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Implementation of functors and processors, based on static_variant
 * 
 * @copyright Copyright (c) 2021
 * 
*/

/**
 * @example "functor ~ usage"
 * 
 * chaining:
 * ```
 * struct do_nothing_1 : public functor<do_nothing begin>;
 * struct do_nothing_2 : public functor<do_nothing_1>;
 * struct do_nothing_3 : public functor<do_nothing_2>;
 * struct do_nothing_4 : public functor<do_nothing_3>;
 * ```
 * 
 * creating chain:
 * ```
 * using functors = functor_collection_t<do_nothing_4>;
 * ```
 * 
 * which is alternative to:
 * ```
 * std::variant<
 *  	do_nothing_begin, 
 *  	do_nothing_1, 
 *  	do_nothing_2, 
 *  	do_nothing_3, 
 *  	do_nothing_4, 
 *  	do_nothing_end
 * >
 * ```
*/

#pragma once

// Project includes
#include "recursive_type_collection.hpp"
#include "visitor.hpp"

// STL
#include <concepts>
#include <cassert>
#include <variant>
#include <functional>

namespace patterns
{
	/**
	 * @brief contains definition for functors, that are stored in static_variants
	 */
	namespace functors
	{
		using patterns::visitable;

		/**
		 * @brief provides basic interface for defining functor
		 * 
		 * @tparam return_t what is returned after invokation
		 * @tparam arg_t variadic type of arguments
		 */
		template <typename return_t = void, typename... arg_t>
		struct _functor : public visitable<_functor<return_t, arg_t...>>
		{
			/**
			 * @brief This should be overrided in definition
			 */
			virtual return_t operator()(arg_t...){};
		};

		/**
		 * @brief Adds possibiblity to iterate over static_variant
		 * 
		 * @tparam _Prev points to previously created functor
		 * @tparam return_t type of returned value
		 * @tparam arg_t optional arguments
		 */
		template <typename _Prev, typename return_t = void, typename... arg_t>
		struct functor : public _functor<return_t, arg_t...>
		{
			using Prev = _Prev;
		};

		/**
		 * @brief this is helper class for creating type collections, whichc is putted at the end
		 * 
		 * @tparam _Last type of last item in type collection
		 */
		template <typename _Last>
		struct do_nothing_functor_end : public functor<_Last> {};

		/**
		 * @brief this is helper class for creating type collections, whichc is putted at the begining
		 */
		struct do_nothing_functor_begin : _functor<>{};

		/**
		 * @brief created static_variant for chain of functors
		 * 
		 * @tparam _Last last functor
		 */
		template <typename _Last>
		using functor_collection_t = RCNS::recursive_concentrator<std::variant, do_nothing_functor_begin, do_nothing_functor_end<_Last>>::result;

	} // namespace functors

	namespace processors
	{
		namespace processor_concept_ns
		{
			template <typename T>
			using c_func = decltype([](T) {});

			using Supp = c_func<int>;
			using variadic_t = std::variant<Supp, c_func<float>>;

			template <template <typename _Supp> typename T>
			concept is_valid_processor_t = requires(T<Supp> var)
			{
				{
					var.invoke(variadic_t{[](int) {}})
				}
				->std::same_as<void>;
			};
		} // namespace processor_concept_ns

		using processor_concept_ns::is_valid_processor_t;

		template <typename _Supported>
		struct processor
		{
			virtual void invoke(_Supported &&op) = 0;
		};

		template <typename _Supported>
		struct processor_autocall : public processor<_Supported>
		{
			// default call
			virtual void invoke(_Supported &&op) override
			{
				op();
			}
		};

	} // namespace processors

} // namespace patterns