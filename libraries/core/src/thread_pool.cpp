#include <core/thread_pool.h>

void _thread_pool_impl::cleanup()
{
    cleanup_threads([](const thread_with_status& th){ return th.worker.get_flag(1);});
}

void _thread_pool_impl::forced_cleanup()
{
    cleanup_threads([](const thread_with_status& th){ return true;});
}

_thread_pool_impl::~_thread_pool_impl()
{
    forced_cleanup();
}

void _thread_pool_impl::cleanup_threads(const cleanup_function_t _pred)
{
    threads.remove_if( _pred );
}
