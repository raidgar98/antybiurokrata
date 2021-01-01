#pragma once

#include <memory>
#include <cassert>
#include <concepts>
#include <atomic>

#include "observer.hpp"

namespace patterns
{
	namespace singleton_wrappers
	{
		template< typename T>
		concept wrapped_item_req = requires{
			std::is_move_constructible<T>::value;
			std::is_destructible<T>::value;
		};

		template <template <typename _Wrapper> typename _WrapperSetter, template <wrapped_item_req T> typename Wrapper>
		concept wrapper_setter_req = requires(Wrapper<int> w)
		{
			// opeartor() should set value into wrapper
			{_WrapperSetter<Wrapper<int>>{}(w, int{3})};
		};

		template <template <typename _Wrapper> typename _WrapperGetter, template <wrapped_item_req T> typename Wrapper>
		concept wrapper_getter_req = requires(Wrapper<int> w)
		{
			// operator() should return T& of stored value in Wrapper
			{
				_WrapperGetter<Wrapper<int>>{}(w)
			}
			->std::same_as<int &>;
		};

		template <template <typename _Wrapper> typename _WrapperIsEmpty, template <wrapped_item_req T> typename Wrapper>
		concept wrapper_is_empty_req = requires(Wrapper<int> w)
		{
			// operator() should return bool; true - if is empty, and false if it's storing something
			{
				_WrapperIsEmpty<Wrapper<int>>{}(w)
			}
			->std::same_as<bool>;
		};

		template< template<wrapped_item_req T> typename Wrapper, wrapped_item_req T >
		struct wrapper_setter_base
		{
			void operator()(Wrapper<T>& _wrapper, T& _value) const { set(_wrapper, _value); }
		protected:
			virtual void set(Wrapper<T>& _wrapper, T& _value) const = 0;
		};

		template< template<wrapped_item_req T> typename Wrapper, wrapped_item_req T >
		struct wrapper_getter_base
		{
			T& operator()(Wrapper<T>& _w) const { return get(_w); }
		protected:
			virtual T& get(Wrapper<T>& _wrapper) const = 0;
		};

		template< typename Wrapper >
		struct wrapper_is_empty_base
		{
			bool operator()(const Wrapper& _w) const { return empty(_w); }
		protected:
			virtual bool empty(const Wrapper& _wrapper) const = 0;
		};

		// Uniq ptr

		template<wrapped_item_req T>
		struct uniq_ptr_set : public wrapper_setter_base<std::unique_ptr<T>, T>
		{
			using wrapper_setter_base<std::unique_ptr<T>, T>::wrapper_setter_base;
		protected:
			virtual void set(std::unique_ptr<T>& ptr, T& p) const override
			{
				ptr.reset( new T{ std::move(p) } );
			}
		};

		template<wrapped_item_req T>
		struct uniq_ptr_get : public wrapper_getter_base<std::unique_ptr<T>, T>
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

		// Atomic

		template<wrapped_item_req _T, typename T = std::unique_ptr<_T>>
		struct atomic_set : public wrapper_setter_base<std::atomic<T>, T>
		{
			using wrapper_setter_base<std::atomic<T>, T>::wrapper_setter_base;
		protected:
			virtual void set(std::atomic<std::unique_ptr<_T>>& w, T& p) const override
			{
				w.store(std::move( T( new _T{ std::move( p ) } ) ));
			}
		};

		template<wrapped_item_req _T, typename T = std::unique_ptr<_T>>
		struct atomic_get : public wrapper_getter_base<std::atomic<T>, T>
		{
			using wrapper_getter_base<std::atomic<T>, T>::wrapper_getter_base;
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
			virtual bool empty(std::atomic<T>& ptr) const override
			{
				return w.load().get() == nullptr;
			}
		};


	} // namespace singleton_wrappers

	using namespace singleton_wrappers;

	// If you need more singletons of same type, change second parameter of template
	template <
		wrapped_item_req T,
		typename _ = void,
		typename _WrapperType = std::unique_ptr<T>,
		typename _WrapperGetter = uniq_ptr_set<T>,
		typename _WrapperSetter = uniq_ptr_get<T>,
		typename _WrapperIsEmpty = uniq_ptr_is_empty<T>
	>
	class _singleton_impl
	{
	private:

		using _singl_t = _singleton_impl<T, _, _WrapperType, _WrapperGetter, _WrapperSetter, _WrapperIsEmpty>;

		struct singleton_notify_deleter
		{
			void operator()(_WrapperType* w) const
			{
				assert(w != nullptr);
				_singl_t::on_delete( _WrapperGetter{}(*w) );
				delete w;
			}
		};

		inline static std::unique_ptr<_WrapperType, singleton_notify_deleter > __m_wrapped_item{ new _WrapperType{}, singleton_notify_deleter{} };

	public:

		inline static patterns::observable<T&, singleton_notify_deleter > on_delete;

		static void set(T& item)
		{
			_WrapperSetter{}( *__m_wrapped_item, item);
		}

		static T &get()
		{
			return _WrapperGetter{}( *__m_wrapped_item);
		}

		static bool empty()
		{
			return _WrapperIsEmpty{}( *__m_wrapped_item);
		}

		using value_type = T;
	};

	template<typename T, typename _ = void>
	using simple_singleton = _singleton_impl<T, _>;

	template<typename T, typename _ = void>
	using thread_safe_singleton = _singleton_impl<
		T,
		_,
		std::atomic<T>,
		atomic_set<T>,
		atomic_get<T>,
		atomic_is_empty<T>
	>;

} // namespace patterns