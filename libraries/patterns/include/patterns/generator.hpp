// Source: https://en.cppreference.com/w/cpp/coroutine/coroutine_handle#Example
#pragma once

#include <coroutine>
#include <iostream>
#include <optional>

namespace patterns
{

    /*
// example usage:

    template<std::integral T>
    generator<T> range(T first, const T last) {
        while (first < last)
            co_yield first++;
    }
*/
    template <std::movable T>
    class generator
    {
    public:
        struct promise_type
        {
            generator<T> get_return_object()
            {
                return generator{Handle::from_promise(*this)};
            }
            static std::suspend_always initial_suspend() noexcept
            {
                return {};
            }
            static std::suspend_always final_suspend() noexcept
            {
                return {};
            }
            std::suspend_always yield_value(T value) noexcept
            {
                current_value = std::move(value);
                return {};
            }
            // Disallow co_await in generator coroutines.
            void await_transform() = delete;
            [[noreturn]] static void unhandled_exception()
            {
                throw;
            }

            std::optional<T> current_value;
        };

        using Handle = std::coroutine_handle<promise_type>;

        explicit generator(const Handle coroutine) : m_coroutine{coroutine}
        {
        }

        generator() = default;
        ~generator()
        {
            if (m_coroutine)
            {
                m_coroutine.destroy();
            }
        }

        generator(const generator &) = delete;
        generator &operator=(const generator &) = delete;

        generator(generator &&other) noexcept : m_coroutine{other.m_coroutine}
        {
            other.m_coroutine = {};
        }
        generator &operator=(generator &&other) noexcept
        {
            if (this != &other)
            {
                if (m_coroutine)
                {
                    m_coroutine.destroy();
                }
                m_coroutine = other.m_coroutine;
                other.m_coroutine = {};
            }
            return *this;
        }

        // Range-based for loop support.
        class gen_iterator
        {
        public:
            void operator++()
            {
                m_coroutine.resume();
            }
            const T &operator*() const
            {
                return *m_coroutine.promise().current_value;
            }
            bool operator==(std::default_sentinel_t) const
            {
                return !m_coroutine || m_coroutine.done();
            }

            explicit gen_iterator(const Handle coroutine) : m_coroutine{coroutine}
            {
            }

        private:
            Handle m_coroutine;
        };

        gen_iterator begin()
        {
            if (m_coroutine)
            {
                m_coroutine.resume();
            }
            return gen_iterator{m_coroutine};
        }
        std::default_sentinel_t end()
        {
            return {};
        }

    private:
        Handle m_coroutine;
    };

} // namespace patterns