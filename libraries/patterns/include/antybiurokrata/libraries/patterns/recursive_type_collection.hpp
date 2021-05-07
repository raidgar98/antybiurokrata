/**
 * @file recursive_type_collection.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief This allows to convert linked types to any type collections like tuples or variants
 * 
 * @inspiration: https://stackoverflow.com/a/52393977
 */

#pragma once

namespace patterns
{
	/**
	 * @brief recursive concentrator namespace
	 */
	namespace RCNS
	{
		/**
		 * @brief verifies is given type iterable
		 * 
		 * @tparam T tpye to check
		 */
		template<typename T> concept recursive_concentrator_c = requires { typename T::Prev; };


		/**
		 * @brief core implementation of type concentrator
		 * 
		 * @param _Storage output collection of types [ Ex: std::variant, std::tuple ]
		 * @param _First first type in _Storage
		 * @param LastElement last type in _Storage, it has to fullfill `recursive_concentrator_c`
		 */
		template<template<typename...> class _Storage, typename _First /* can be empty struct */,
					recursive_concentrator_c LastElement>
		struct recursive_concentrator
		{
			template<typename T, typename... Args> struct concatenator;

			template<recursive_concentrator_c FArg, typename... Args> struct concatenator<_Storage<FArg, Args...>>
			{
				using type = typename concatenator<_Storage<typename FArg::Prev, FArg, Args...>>::type;
			};

			template<typename... Args> struct concatenator<_Storage<_First, Args...>>
			{
				using type = _Storage<_First, Args...>;
			};

			using result = concatenator<_Storage<LastElement>>::type;
		};

	}	 // namespace RCNS

}	 // namespace patterns