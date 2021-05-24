/**
 * @file global_adapters.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief constains declarations of (network) global varriables
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>

namespace core
{
	namespace network
	{
		/** @brief contains global varriables with diffrent adapters */
		namespace global_adapters
		{
			extern bgpolsl_adapter polsl;
			extern orcid_adapter orcid;
			extern scopus_adapter scopus;

		}	 // namespace global_adapters
	}		 // namespace network
}	 // namespace core