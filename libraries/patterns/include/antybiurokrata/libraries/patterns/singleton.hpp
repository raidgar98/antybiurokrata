/**
 * @file singleton.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Implementation of various singletons
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @example "singleton ~ usage"
 * 
 * creating singleton:
 * 
 * ```
 * using my_singleton = simple_singleton< my_type >; // second param: 0 by default
 * using my_singleton_1 = simple_singleton< my_type, 1>;
 * using my_singleton_2 = simple_singleton< my_type, 2>;
 * using my_singleton_other = simple_singleton< other_type >;
 * ```
 */

#pragma once

// STL
#include <memory>
#include <cassert>
#include <concepts>
#include <atomic>

// Project Includes
#include "observer.hpp"

namespace patterns
{
	namespace singleton_wrappers
	{
		/**
		 * @brief requirements for wrapper
		 * 
		 * @tparam T any item
		 */
		template<typename T> concept wrapped_item_req = requires
		{
			std::is_move_constructible<T>::value;
			std::is_destructible<T>::value;
		};

		/**
		 * @brief opeartor() should set value into wrapper
		 * 
		 * @tparam _WrapperSetter functor that changes wrapper content
		 * @tparam Wrapper any wrapper
		 */
		template<template<typename _Wrapper> typename _WrapperSetter,
					template<wrapped_item_req T> typename Wrapper>
		concept wrapper_setter_req = requires(Wrapper<int> w)
		{
			{_WrapperSetter<Wrapper<int>>{}(w, int{3})};
		};

		/** 
		 * @brief operator() should return T& of stored value in Wrapper 
		 * 
		 * @tparam _WrapperGetter functor that returns wrapper content
		 * @tparam Wrapper any wrapper
		 */
		template<template<typename _Wrapper> typename _WrapperGetter,
					template<wrapped_item_req T> typename Wrapper>
		concept wrapper_getter_req = requires(Wrapper<int> w)
		{
			{
				_WrapperGetter<Wrapper<int>>{}(w)
			}
			->std::same_as<int&>;
		};

		/**
		 * @brief operator() should return bool; true - if is empty, and false if it's storing something
		 * 
		 * @tparam _WrapperIsEmpty check is wrapper empty
		 * @tparam Wrapper any wrapper
		 */
		template<template<typename _Wrapper> typename _WrapperIsEmpty,
					template<wrapped_item_req T> typename Wrapper>
		concept wrapper_is_empty_req = requires(Wrapper<int> w)
		{
			{
				_WrapperIsEmpty<Wrapper<int>>{}(w)
			}
			->std::same_as<bool>;
		};

		/**
		 * @brief interface for all wrapper setters
		 * 
		 * @tparam Wrapper any wrapper
		 * @tparam T wrapped type
		 */
		template<template<wrapped_item_req T> typename Wrapper, wrapped_item_req T>
		struct wrapper_setter_base
		{
			/**
			 * @brief proxy to set methode
			 * 
			 * @param _wrapper wrapper that will be modified
			 * @param _value value to set
			 */
			void operator()(Wrapper<T>& _wrapper, T& _value) const { set(_wrapper, _value); }

		 protected:
			/**
			 * @brief this methode have to be overloaded
			 * 
			 * @param _wrapper wrapper to modify
			 * @param _value value to set
			 */
			virtual void set(Wrapper<T>& _wrapper, T& _value) const = 0;
		};

		/**
		 * @brief interface for all wrapper getters
		 * 
		 * @tparam Wrapper any wrapper
		 * @tparam T wrapped type
		 */
		template<template<wrapped_item_req T> typename Wrapper, wrapped_item_req T>
		struct wrapper_getter_base
		{
			/**
			 * @brief proxy to get methode
			 * 
			 * @param _w wrapper that will be accessed
			 * @return T& value from given wrapper
			 */
			T& operator()(Wrapper<T>& _w) const { return get(_w); }

		 protected:
			/**
			 * @brief both of these methodes, have to be overloaded
			 * 
			 * @param _w wrapper that will be accessed
			 * @return T& value from given wrapper
			 */
			virtual T& get(Wrapper<T>& _wrapper) const = 0;
		};

		/**
		 * @brief interface for all empty checkers
		 * 
		 * @tparam Wrapper any specialized wrapper
		 */
		template<typename Wrapper> struct wrapper_is_empty_base
		{
			/**
			 * @brief proxy to empty methode
			 * 
			 * @param _w any wrapper
			 * @return true if given wrapper is empty
			 * @return false if given wrapper is set to something
			 */
			bool operator()(const Wrapper& _w) const { return empty(_w); }

		 protected:
			/**
			 * @brief this methode have to be overloaded
			 * 
			 * @param _wrapper wrapper to check
			 * @return true if given wrapper is empty
			 * @return false if given wrapper is set to something
			 */
			virtual bool empty(const Wrapper& _wrapper) const = 0;
		};

		/**
		 * @brief whole implementation of uniq_ptr based wrapper
		 */
		namespace uniq_ptr_w
		{
			template<wrapped_item_req T>
			struct uniq_ptr_set : public wrapper_setter_base<std::unique_ptr, T>
			{
				using wrapper_setter_base<std::unique_ptr, T>::wrapper_setter_base;

			 protected:
				virtual void set(std::unique_ptr<T>& ptr, T& p) const override
				{
					ptr.reset(new T{std::move(p)});
				}
			};

			template<wrapped_item_req T>
			struct uniq_ptr_get : public wrapper_getter_base<std::unique_ptr, T>
			{
			 protected:
				virtual T& get(std::unique_ptr<T>& ptr) const override
				{
					assert(ptr.get());
					return *ptr;
				}
			};

			template<wrapped_item_req T>
			struct uniq_ptr_is_empty : public wrapper_is_empty_base<std::unique_ptr<T>>
			{
			 protected:
				virtual bool empty(std::unique_ptr<T>& ptr) const override
				{
					return ptr.get() == nullptr;
				}
			};
		}	 // namespace uniq_ptr_w

		/**
		 * @brief whole implementation of std::atomic based wrapper
		 */
		namespace atomic_w
		{
			template<wrapped_item_req _T, typename T = std::unique_ptr<_T>>
			struct atomic_set : public wrapper_setter_base<std::atomic, T>
			{
				using wrapper_setter_base<std::atomic, T>::wrapper_setter_base;

			 protected:
				virtual void set(std::atomic<std::unique_ptr<_T>>& w, T& p) const override
				{
					w.store(std::move(T(new _T{std::move(p)})));
				}
			};

			template<wrapped_item_req _T, typename T = std::unique_ptr<_T>>
			struct atomic_get : public wrapper_getter_base<std::atomic, T>
			{
				using wrapper_getter_base<std::atomic, T>::wrapper_getter_base;

			 protected:
				virtual T& get(std::atomic<T>& w) const override
				{
					assert(w.load().get());
					return *w.load();
				}
			};

			template<wrapped_item_req T>
			struct atomic_is_empty : public wrapper_is_empty_base<std::atomic<T>>
			{
				using wrapper_is_empty_base<std::atomic<T>>::wrapper_is_empty_base;

			 protected:
				virtual bool empty(const std::atomic<T>& ptr) const override
				{
					return ptr.load().get() == nullptr;
				}
			};
		}	 // namespace atomic_w
	}		 // namespace singleton_wrappers

	using namespace singleton_wrappers;
	using namespace uniq_ptr_w;

	//
	/**
	 * @brief core implementaion of singleton
	 * @note If you need more singletons with same type, change second parameter of template
	 * 
	 * @tparam T type to hold by singleton
	 * @tparam _ dummy value
	 * @tparam _WrapperType wrapper to use to hold T
	 * @tparam _WrapperGetter getter, compatibile with _WrapperType
	 * @tparam _WrapperSetter setter, compatibile with _WrapperType
	 * @tparam _WrapperIsEmpty empty checker, compatibile with _WrapperType
	 */
	template<wrapped_item_req T, uint64_t _ = 0, typename _WrapperType = std::unique_ptr<T>,
				typename _WrapperGetter = uniq_ptr_set<T>, typename _WrapperSetter = uniq_ptr_get<T>,
				typename _WrapperIsEmpty = uniq_ptr_is_empty<T>>
	class _singleton_impl
	{
	 private:
		using _singl_t
			 = _singleton_impl<T, _, _WrapperType, _WrapperGetter, _WrapperSetter, _WrapperIsEmpty>;

		/**
		 * @brief functor, responsible for notifying all users of this singleton, if they care of course
		 */
		struct singleton_notify_deleter
		{
			/**
			 * @brief this is called by unique_ptr
			 * 
			 * @param w currently destroyed value
			 */
			void operator()(_WrapperType* w) const
			{
				assert(w != nullptr);
				_singl_t::on_delete(_WrapperGetter{}(*w));
				std::default_delete<_WrapperType>{}(w);
			}
		};

		/**
		 * @brief holder of data
		 */
		inline static std::unique_ptr<_WrapperType, singleton_notify_deleter> __m_wrapped_item{
			 new _WrapperType{},
			 singleton_notify_deleter{}};

	 public:
		/**
		 * @brief signal that can be subscribed by anyone, who need to knopw when this singleton is destroyed
		 */
		inline static patterns::observable<T&, singleton_notify_deleter> on_delete;

		/**
		 * @brief sets value holded by this singleton
		 * @note on change, memory will be automatically released, so do not put any references here
		 * @param item value to set
		 */
		static void set(T* item) { _WrapperSetter{}(*__m_wrapped_item, *item); }

		/**
		 * @brief returns reference to currently holding value
		 * 
		 * @return T& value
		*/
		static T& get() { return _WrapperGetter{}(*__m_wrapped_item); }

		/**
		 * @brief returns is singleton empty or not
		 * 
		 * @return true if empty
		 * @return false if not empty
		*/
		static bool empty() { return _WrapperIsEmpty{}(*__m_wrapped_item); }

		using value_type = T;
	};

	namespace type_enumerator
	{
		template<uint64_t N_> struct N
		{
			constexpr static uint64_t value{N_};
		};
	};	  // namespace type_enumerator

	/**
	 * @brief aliasing for easier usage
	 * 
	 * @tparam T data to store
	 * @tparam _ dummy value
	 */
	template<typename T, uint64_t _ = 0> using simple_singleton = _singleton_impl<T, _>;

	/**
	 * @brief threadsafe singleton, that uses std::atomic as container
	 * 
	 * @tparam T value to store
	 * @tparam _ dummy value
	 */
	template<typename T, uint64_t _ = 0>
	using thread_safe_singleton
		 = _singleton_impl<T, _, std::atomic<T>, typename atomic_w::atomic_set<T>,
								 typename atomic_w::atomic_get<T>, typename atomic_w::atomic_is_empty<T>>;

}	 // namespace patterns