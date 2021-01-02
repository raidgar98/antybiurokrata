#pragma once

// Project includes
#include "recursive_type_collection.hpp"

// STL
#include <concepts>
#include <cassert>
#include <variant>
#include <functional>

namespace patterns
{
    namespace functors
    {
        template <typename return_t = void, typename... arg_t>
        struct _functor
        {
            virtual return_t operator()(arg_t...){};
        };

        template <typename _Prev /* points to previously created functor */, typename return_t = void, typename... arg_t>
        struct functor : public _functor<return_t, arg_t...>
        {
            using Prev = _Prev;
        };

        template <typename _Last>
        struct do_nothing_functor_end : public functor<_Last>
        {
        };
        struct do_nothing_functor_begin : _functor<>
        {
        };

        template <typename _Last>
        using functor_collection_t = RCNS::recursive_concentrator<std::variant, do_nothing_functor_begin, do_nothing_functor_end<_Last>>::result;

    } // namespace functors

    namespace processors
    {
        template <class variadic_t /*Ex. std::variant*/>
        struct processor_base
        {
            processor_base *get() { return this; }
            virtual bool is_my_type(variadic_t &) const = 0;
            virtual void invoke(variadic_t &) = 0;
        };

        namespace processor_concept_ns
        {
            template<typename T>
            using c_func = decltype([](T){});

            using Supp = c_func<int>;
            using variadic_t = std::variant<Supp, c_func<float>>;

            template <template <class v_t, typename _Supp> typename T>
            concept is_valid_processor_t = requires(T <variadic_t, Supp> var)
            {
                std::is_base_of<processor_base<variadic_t>, T<variadic_t, Supp>>::value;
                { var.is_my_type( variadic_t{ [](int){} } ) };
                { var.invoke( variadic_t{ [](int){} } ) };
            };
        } // namespace processor_concept_ns
        
        using processor_concept_ns::is_valid_processor_t;

        template <class variadic_t, typename _Supported>
        struct processor : public processor_base<variadic_t>
        {
            // not sofisticated, but looks, and works fine
            virtual bool is_my_type(variadic_t &op) const override
            {
                return std::get_if<_Supported>(&op);
            }

            virtual void invoke(variadic_t &op) = 0;
        };

        template <class variadic_t, typename _Supported>
        struct processor_autocall : public processor<variadic_t, _Supported>
        {
            // default call
            virtual void invoke(variadic_t &op) override
            {
                std::get<_Supported>(op)();
            }
        };

        // if you have collection of processors it's good to put it at end of collection
        template <class variadic_t>
        struct not_supported_operation_type_processor : public processor_base<variadic_t>
        {
            virtual bool is_my_type(variadic_t &op) const override
            {
                assert(false);
                return true;
            }

            [[noreturn]] virtual void invoke(variadic_t &op) override
            {
                assert(false);
            }
        };
    } // namespace processors

} // namespace patterns