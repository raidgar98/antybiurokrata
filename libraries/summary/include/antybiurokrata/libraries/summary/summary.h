/**
 * @file summary.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contatins definitions of mechanism to compare results
 * @version 0.1
 * @date 2021-05-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/orm/orm.h>

namespace core
{
	namespace reports
	{
		/**
		 * @brief matches given publications and produces summary
		 */
		// class summary : public Log<summary>
		// {
		// 	using Log<summary>::log;
		// 	using core::orm::publication_storage_t;
		// 	using second_publications_t = std::optional< std::ref<publication_storage_t> >;

		// 	const publication_storage_t& m_reference;
		// 	second_publications_t m_second{ std::nullopt };

		// public:

		// 	explicit summary(const publication_storage_t& reference, const second_publications_t& compare_with = std::nullopt);

		// };

	}	 // namespace reports
}	 // namespace core