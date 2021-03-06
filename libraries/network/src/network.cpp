#include <antybiurokrata/libraries/network/network.h>

namespace core
{
	namespace network
	{
		namespace detail
		{
			loop_holder_t global_loop{};
		}

		connection_handler::connection_handler(const str_v& url, const bool detached)
		{
			using namespace network::detail;
			if(detached) loop = std::make_shared<loop_holder_t>();
			else
			{
				loop = std::shared_ptr<loop_holder_t>{&global_loop, [](loop_holder_t*) {}};
			}
			log.info() << "setting up connection with host: `" << url << "`" << logger::endl;
			this->connection = drogon::HttpClient::newHttpClient(url.data(), loop->handle.get());

			check_nullptr{this->connection};
		}

		connection_handler::raw_response_t connection_handler::send_request(
			 connection_handler::raw_request_t request)
		{
			check_nullptr{this->connection};
			return this->connection->sendRequest(request);
		}
	}	 // namespace network
}	 // namespace core
