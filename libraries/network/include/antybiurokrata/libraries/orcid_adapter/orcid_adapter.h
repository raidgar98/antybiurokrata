/**
 * @file orcid_adapter.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of network handler for connecting with orcid api
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
		/** @brief data collector for orcid */
		struct orcid_adapter : protected connection_handler, private Log<orcid_adapter>
		{
			using Log<orcid_adapter>::log;
			using value_t	= std::list<detail::json_repr_t>;
			using result_t = std::shared_ptr<value_t>;

			/** @brief default constructor */
			orcid_adapter() : connection_handler{"https://pub.orcid.org", true} {}

			/**
			 * @brief get the result from orcid for given orcid string
			 * 
			 * @param orcid string in format that maatches regex: ([0-9]{4})-\1-\1-\1
			 * @return result_t list of trival object representation
			 */
			[[nodiscard]] result_t get_person(const str& orcid);

		 private:
			/**
			 * @brief prepares request for given orcid string (headers, paths, etc...)
			 * 
			 * @param orcid string
			 * @return drogon::HttpRequestPtr 
			 */
			drogon::HttpRequestPtr prepare_request(const str& orcid);
		};
	}	 // namespace network
}	 // namespace core