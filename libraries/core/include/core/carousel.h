#pragma once

// STL
#include <memory>
#include <vector>

// Project includes
#include <logger/logger.h>
#include "threaded_processor.h"

namespace core
{

    template<typename T>
    concept is_threated_processor_based = requires{
        std::is_base_of_v< threaded_processor_base, T > || std::is_same_v<threaded_processor_base, T>;
    };

    class corousel : public Log<corousel>
    {

        template <typename T> using collection_t = std::vector<T>;
        template<typename T> using storage_unit_t = std::shared_ptr<T>;

        collection_t < storage_unit_t< threaded_processor_base > > carts;

        // not copyable
        corousel(const corousel&) = delete;
        corousel& operator=(const corousel&) = delete;

        template<is_threated_processor_based T>
        void register_cart( T * ptr )
        {
            assert( ptr );
            carts.emplace_back( static_cast<threaded_processor_base*>(ptr) );
        }

        template<typename T>
        void put(T& obj)
        {
            for( threaded_processor_base* cart : carts )
                if( cart->is_supported(obj) )
                {
                    cart->start(obj);
                    return true;
                }
        }
    };

} // namespace core