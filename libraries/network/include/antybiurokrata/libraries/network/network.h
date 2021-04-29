#pragma once

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/types.hpp>

// drogon
#include <drogon/drogon.h>

namespace core
{
	namespace network
	{
		namespace detail
		{
			struct loop_holder_t
			{
				using event_loop_t = typename trantor::EventLoop;

				std::unique_ptr<event_loop_t> handle;
				std::unique_ptr<std::jthread> thread;

				loop_holder_t()
					: handle{ new event_loop_t{} }
				{
					thread = std::make_unique<std::jthread>(
						[&] {
							handle->moveToCurrentThread();
							handle->loop();
						}
					);
				}

				~loop_holder_t() { handle->quit(); }
			};
		}

		class connection_handler : public Log<connection_handler>
		{
			using Log<connection_handler>::log;
			using raw_response_t = std::pair<drogon::ReqResult, drogon::HttpResponsePtr>;
			using raw_request_t = drogon::HttpRequestPtr;

			detail::loop_holder_t loop;
			drogon::HttpClientPtr connection;

		public:

			explicit connection_handler(const str_v& url);
			raw_response_t send_request(raw_request_t);

			virtual ~connection_handler() {}
		};
	}
}