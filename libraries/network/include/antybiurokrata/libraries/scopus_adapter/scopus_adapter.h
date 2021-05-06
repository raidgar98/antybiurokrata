/**
 * @file scopus_adapter.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of network handler for connecting with scopus
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/network/network.h>
#include <antybiurokrata/libraries/patterns/visitor.hpp>


namespace core
{
	namespace network
	{
		namespace detail
		{
			/** @brief trival object representation of scopus output */
			struct scopus_repr_t : Log<scopus_repr_t>, public patterns::visitable<scopus_repr_t>
			{
				using Log<scopus_repr_t>::log;

				u16str orcid{};
				u16str year{};
				u16str title{};
				u16str translated_title{};
				std::vector<std::pair<u16str, u16str>> ids{};

				/** @brief DEBUG */
				void print() const
				{
					log << "orcid: " << orcid << logger::endl;
					log << "year: " << year << logger::endl;
					log << "title: " << title << logger::endl;
					if (!translated_title.empty()) log << "translated_title: " << translated_title << logger::endl;
					for (const auto &x : ids) log << "id: ( " << x.first << " ; " << x.second << " )" << logger::endl;
				}
			};
		}

		/** @brief data collector for scopus */
		struct scopus_adapter : protected connection_handler, private Log<scopus_adapter>
		{
			using Log<scopus_adapter>::log;
			using value_t = std::list<detail::scopus_repr_t>;
			using result_t = std::shared_ptr<value_t>;

			/** @brief default constructor */
			scopus_adapter() : connection_handler{"https://api.elsevier.com", true} {}

			/**
			 * @brief get the result from scopus for given orcid string
			 * 
			 * @param orcid string in format that maatches regex: ([0-9]{4})-\1-\1-\1
			 * @return result_t list of trival object representation
			 */
			[[nodiscard]] result_t get_person(const str &orcid);

		private:

			/**
			 * @brief prepares request for given orcid string (headers, paths, etc...)
			 * 
			 * @param orcid string
			 * @param offset size_t how much to move from 0 index [= 0]
			 * @param count size_t amount of items to retreive (recommended and bydefault [= 25])
			 * @return drogon::HttpRequestPtr 
			 */
			drogon::HttpRequestPtr prepare_request(const str& orcid, const size_t offset = 0ul, const size_t count = 25ul);
		};
	}
}