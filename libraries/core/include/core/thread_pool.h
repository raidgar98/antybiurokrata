#pragma once

/*

singleton:
-> takes new threads and runs it
-> finishes them in 2 cases:
    -> thread finishes
    -> gets request to stop

*/

// Project includes
#include <sneaky_pointer/sneaky_pointer.hpp>
#include <patterns/singleton.hpp>
#include <logger/logger.h>

// STL
#include <thread>
#include <list>
#include <concepts>

namespace core
{

    struct thread_with_status : Log<thread_with_status>
    {
        using Log<thread_with_status>::log;
        using thread_t = std::jthread;

        sneaky_pointer<thread_t, 1 /*is_ready*/> worker;

        template <typename FuncT, typename ... Args>
        explicit thread_with_status(const FuncT fun, Args&& ... argv)
        {
            clean_worker();
            worker.set_pointer(new thread_t([this, fun](Args&& ... vargs) -> void {
                try
                {
                    fun(std::forward<Args>(vargs)...);
                }
                catch (const std::exception &e)
                {
                    this->log.warn("Failed to execute: " + log.get_class_name<FuncT>());
                    this->log.error("catched std::exception: " + e.what());
                    this->log.print_stacktrace();
                }
                catch (...)
                {
                    this->log.warn("Failed to execute: " + log.get_class_name<FuncT>());
                    this->log.error("catched unknown exception");
                    this->log.print_stacktrace();
                }
                this->worker.set_flag(1, true);
            }, std::forward<Args>(argv)... ));
        }

        ~thread_with_status()
        {
            clean_worker();
        }

        void clean_worker()
        {
            worker.set_flag(1, false);
            thread_t *th = worker.get_pointer();
            if (th)
            {
                worker.set_pointer(nullptr);
                delete th;
                th = nullptr;
            }
        }
    };

    class _thread_pool_impl
    {

        using collection_t = std::list<thread_with_status>;
        using cleanup_function_t = std::function<bool(const thread_with_status &)>;

        collection_t threads;

    public:
        template <typename FuncT, typename ... Args>
        void push(const FuncT fun, Args&& ... argv )
        {
            threads.emplace_back<FuncT>(fun, std::forward<Args>(argv)...);
            cleanup();
        }

        void cleanup();
        ~_thread_pool_impl();

    private:
        void cleanup_threads(const cleanup_function_t);
        void forced_cleanup();
    };

    // Singleton
    using thread_pool = patterns::thread_safe_singleton<_thread_pool_impl>;

} // namespace core