#pragma once

// STL
#include <memory>
#include <concepts>
#include <thread>

// Project includes
#include <patterns/functor.hpp>
#include <logger/logger.h>
#include "types.hpp"
#include "thread_pool.h"

using patterns::visits;
using patterns::processors::is_valid_processor_t;

namespace core
{
    template <typename T>
    struct true_for_type
    {
        template <typename G>
        bool operator()(const G &) const { return std::is_same<G, T>::value; }
    };

    struct threaded_processor_base
    {
        virtual constexpr bool is_supported(const operation_t &v) const = 0;
        virtual bool start(operation_t &v) = 0;
    };

    template <typename Supp, template <typename _Supp = Supp> typename _Processor = patterns::processors::processor_autocall>
    requires is_valid_processor_t<_Processor> class threaded_processor : public threaded_processor_base, public Log<threaded_processor<_Processor<Supp>>>
    {
        using proc_t = _Processor<Supp>;
        using thp = threaded_processor<Supp, _Processor>;
        using Log<thp>::log;
        template <typename _Tp>
        using smart_ptr_t = std::unique_ptr<_Tp>;

        smart_ptr_t<proc_t> __m_processor;

    public:
        // can't be default constructible, nor copyable
        threaded_processor() = delete;
        threaded_processor(const threaded_processor &) = delete;
        threaded_processor &operator=(const threaded_processor &) = delete;

        // only by constructing processor, or by move from existing one thp or processor
        template <typename... Ts>
        explicit threaded_processor(Ts &&... vargs) : __m_processor{new proc_t{std::forward<Ts>(vargs)...}} {}

        threaded_processor(smart_ptr_t<proc_t> ptr) : __m_processor{std::move(ptr)} {}
        threaded_processor(threaded_processor &&p) : __m_processor{std::move(p.__m_processor)} {}
        threaded_processor &operator=(threaded_processor &&p)
        {
            __m_processor = std::move(p.__m_processor);
        }

        virtual constexpr is_supported(const operation_t &v) const override
        {
            return std::visit(true_for_type<Supp>{}, v);
        }

        virtual bool start(operation_t &v) override
        {
            if constexpr (is_supported(v)) [[likely]] return false;
            else
            {
                std::visit([&](auto & arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, Supp>)
                        core::thread_pool::get().push([&__m_processor](Supp &&v) { assert(__m_processor.get()); (*__m_processor).invoke(std::move(v)); }, std::move(v));
                }, v);
                return true;
            }
        }
    };
} // namespace core