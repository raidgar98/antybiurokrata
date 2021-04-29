#include <antybiurokrata/libraries/network/network.h>

namespace core
{
    namespace network
    {
        connection_handler::connection_handler(const str_v &url)
            : loop{}
        {
            log.info() << "setting up connection with host: `" << url << "`" << logger::endl;
            this->connection = drogon::HttpClient::newHttpClient(url.data(), loop.handle.get());
            core::exceptions::require<core::exceptions::assert_exception, true>
            { this->connection.get(), "connection shouldn't be nullptr!" };
        }

        connection_handler::raw_response_t connection_handler::send_request
        (connection_handler::raw_request_t request)
        {
            dassert{ this->connection.get(), "connection not set!" };
            return this->connection->sendRequest(request);
        }
    }
}
