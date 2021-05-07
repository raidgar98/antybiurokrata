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
			/** @brief object representation of bg.polsl.pl output */
			struct bgpolsl_repr_t : Log<bgpolsl_repr_t>, public patterns::visitable<bgpolsl_repr_t>
			{
				using Log<bgpolsl_repr_t>::log;

				u16str idt{};
				u16str year{};
				u16str authors{};
				u16str org_title{};
				u16str whole_title{};
				u16str p_issn{};
				u16str doi{};
				u16str e_issn{};
				u16str affiliation{};

				/**
				 * @brief Construct a new bgpolsl repr t object
				 * 
				 * @param words input for preprocessed words 
				*/
				explicit bgpolsl_repr_t(const std::vector<u16str>& words);

				/** @brief DEBUG */
				void print() const;
			};
		}	 // namespace detail

		/** @brief data collector for bg.polsl.pl */
		struct bgpolsl_adapter : protected connection_handler, private Log<bgpolsl_adapter>
		{
			using Log<bgpolsl_adapter>::log;
			using value_t	= std::list<detail::bgpolsl_repr_t>;
			using result_t = std::shared_ptr<value_t>;

			/** @brief default constructor */
			bgpolsl_adapter() : connection_handler{"https://www.bg.polsl.pl", true} {}

			/**
             * @brief get the result from bg.polsl.pl for given name and surname
             * 
             * @param name of author
             * @param surname of author
             * @return result_t list of trival object representation
             */
			[[nodiscard]] result_t get_person(const str_v& name, const str_v& surname);

		 private:
			/**
             * @brief prepares request for Drogon
             * 
             * @param querried_name escaped surname + name 
             * @return drogon::HttpRequestPtr 
             */
			drogon::HttpRequestPtr prepare_request(const str_v& querried_name);
		};
	}	 // namespace network
}	 // namespace core