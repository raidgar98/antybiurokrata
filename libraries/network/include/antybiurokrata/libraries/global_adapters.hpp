#pragma once

#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>

namespace core
{
	namespace network
	{
		namespace global_adapters
		{
			extern bgpolsl_adapter polsl;
			extern orcid_adapter orcid;
			extern scopus_adapter scopus;

		}	 // namespace global_adapters
	}		 // namespace network
}	 // namespace core