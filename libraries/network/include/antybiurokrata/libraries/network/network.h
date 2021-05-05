/**
 * @file network.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains basic implementation of network layer
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/types.hpp>
#include <antybiurokrata/libraries/patterns/singleton.hpp>

// drogon
#include <drogon/drogon.h>

namespace core
{
	namespace network
	{
		namespace detail
		{
			/** @brief encapsulates thread that process requests in loop (from trenor library) */
			struct loop_holder_t : Log<loop_holder_t>
			{
				using event_loop_t = trantor::EventLoop;

				std::unique_ptr<event_loop_t> handle;
				std::unique_ptr<std::jthread> thread;

				loop_holder_t()
					// : handle{ new event_loop_t{} }
				{
					thread = std::make_unique<std::jthread>(
						[&] {
							handle = std::make_unique<event_loop_t>();
							log.info() << "checking is loop in current thread" << logger::endl;
							if(!handle->isInLoopThread()) handle->moveToCurrentThread();
							log.info() << "running loop" << logger::endl;
							handle->loop();
						}
					);
				}

				~loop_holder_t() 
				{ 
					log.info() << "closing loop" << logger::endl;
					handle->quit(); 
				}
			};

			/** @brief global loop for whole program */
			// using global_loop = patterns::thread_safe_singleton<loop_holder_t>;
			extern loop_holder_t global_loop;
		}

		/** @brief provides basic interface for handling http requests */
		class connection_handler : public Log<connection_handler>
		{
			std::shared_ptr<typename detail::loop_holder_t> loop;	/** @brief pointer to loop */
			drogon::HttpClientPtr connection;						/** @brief drogon HTTP connection interface */

		protected:

			using Log<connection_handler>::log;
			using raw_response_t = std::pair<drogon::ReqResult, drogon::HttpResponsePtr>;
			using raw_request_t = drogon::HttpRequestPtr;

		public:

			/**
			 * @brief Construct a new connection handler object
			 * 
			 * @param url url to host
			 * @param detached if set to true, connection will have own thread for execution, if false (default) it will use global loop
			 */
			explicit connection_handler(const str_v& url, const bool detached = false);
			connection_handler() = delete;

			/**
			 * @brief sends given request and returns raw result
			 * 
			 * @return raw_response_t 
			 */
			raw_response_t send_request(raw_request_t);
		};
	}
}