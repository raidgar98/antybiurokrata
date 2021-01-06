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
    namespace functors
    {
        using patterns::Visitable;

        template <typename return_t = void, typename... arg_t>
        struct _functor : public Visitable< _functor<return_t, arg_t...> >
        {
            virtual return_t operator()(arg_t...){};
        };

        template <typename _Prev /* points to previously created functor */, typename return_t = void, typename... arg_t>
        struct functor : public _functor<return_t, arg_t...>
        {
            using Prev = _Prev;
        };

        template <typename _Last>
        struct do_nothing_functor_end : public functor<_Last> {};

        struct do_nothing_functor_begin : _functor<>{};

        template <typename _Last>
        using functor_collection_t = RCNS::recursive_concentrator<std::variant, do_nothing_functor_begin, do_nothing_functor_end<_Last>>::result;

    } // namespace functors

    namespace processors
    {
        namespace processor_concept_ns
        {
            template<typename T>
            using c_func = decltype([](T){});

            using Supp = c_func<int>;
            using variadic_t = std::variant<Supp, c_func<float>>;

            template <template <typename _Supp> typename T>
            concept is_valid_processor_t = requires(T <Supp> var)
            {
                { var.invoke( variadic_t{ [](int){} } ) } -> std::same_as<void>;
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