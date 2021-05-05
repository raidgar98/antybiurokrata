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
#include <antybiurokrata/libraries/patterns/visitor.hpp>

namespace core
{
	namespace network
	{
		namespace detail
		{
			struct orcid_repr_t : Log<orcid_repr_t>, public patterns::visitable<orcid_repr_t>
			{
				using Log<orcid_repr_t>::log;

				u16str year{};
				u16str title{};
				u16str translated_title{};
				std::vector<std::pair<u16str, u16str>> ids{};

				void print() const
				{
					log << "year: " << year << logger::endl;
					log << "title: " << title << logger::endl;
					if (!translated_title.empty())
						log << "translated_title: " << translated_title << logger::endl;
					for (const auto &x : ids)
						log << "id: ( " << x.first << " ; " << x.second << " )" << logger::endl;
					log << logger::endl;
				}
			};
		}

		struct orcid_adapter : protected connection_handler, private Log<orcid_adapter>
		{
			using Log<orcid_adapter>::log;
			using value_t = std::list<detail::orcid_repr_t>;
			using result_t = std::shared_ptr<value_t>;

			orcid_adapter() : connection_handler{"https://pub.orcid.org", true} {}
			[[nodiscard]] result_t get_person(const str &orcid);

		private:
			drogon::HttpRequestPtr prepare_request(const str &orcid);
		};
	}
}