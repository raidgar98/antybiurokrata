#pragma once

// Project includes
#include <patterns/functor.hpp>
#include <logger/logger.h>

// STL
#include <concepts>
#include <stdexcept>

namespace core
{
    namespace operations
    {
        using namespace patterns::functors;

        // Prints pong in console
        struct ping_op : public functor< do_nothing_functor_begin >, private Log<ping_op>
        {
            using Log<ping_op>::log;

            virtual void operator()() override
            {
                log << "Pong!" << logger::endl;
            }
        };
    }

    namespace exceptions
    {
        template<typename T>
        struct exception_base : public std::exception, Log< exception_base<T> >
        {
            const char* ___what;

            virtual const char* what() const noexcept override
            {
                return (log.get_class_name<T>() + ___what).c_str();
            }
        };

        struct exception : public exception_base<exception> {};

        template<typename T>
        concept supported_exception = requires{
            std::is_base_of_v<std::exception, T>;
        };

        struct ____require_base
        {
        protected: 
            static constexpr bool __log_pass{ false };
        };

        template<supported_exception _ExceptionType >
        struct require : Log<require>, ____require_base
        {
            using Log<require>::log;

            template<typename MsgType, typename ... ExceptionArgs>
            require( const bool _check, const MsgType& msg, ExceptionArgs&&... argv)
            {
                if (_check) [[likely]] // be optimist :)
                {
                    if constexpr (__log_pass) log << "passed: `" << msg << '`' << logger::endl;
                }
                else [[unlikely]]
                {
                    log.error("Failed on check");
                    log << msg << logger::endl;
                    log.print_stacktrace();
                    throw _ExceptionType( std::forward<ExceptionArgs>(argv)... );
                }
            }
        };

        struct not_handled_object_exception : public exception_base<not_handled_object_exception> {};
    };

    using operation_t = patterns::functors::functor_collection_t< operations::ping_op /* update this when adding further operations */ >;

} // namespace core