/**
 * @file bgpolsl_adapter.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of network handler for connecting with bg.polsl.pl
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/network/network.h>

namespace core
{
    namespace network
    {
        namespace detail
        {
            struct bgpolsl_repr_t : Log<bgpolsl_repr_t>
            {
                u16str idt{};
                u16str year{};
                u16str authors{};
                u16str org_title{};
                u16str whole_title{};
                u16str e_doc{};
                u16str p_issn{};
                u16str doi{};
                u16str e_issn{};
                u16str affiliation{};

                explicit bgpolsl_repr_t(const std::vector<u16str>&);
                void print() const;
            };
        }

        struct bgpolsl_adapter : protected connection_handler, private Log<bgpolsl_adapter>
        {
            using Log<bgpolsl_adapter>::log;
            using value_t = std::list<detail::bgpolsl_repr_t>;
            using result_t = std::shared_ptr<value_t>;

            bgpolsl_adapter() : connection_handler{"https://www.bg.polsl.pl", true} {}
            [[nodiscard]]
            result_t get_person(const str_v &name, const str_v &surname);

        private:
            drogon::HttpRequestPtr prepare_request(const str_v &querried_name);
        };
    }
}