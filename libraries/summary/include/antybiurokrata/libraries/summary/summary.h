#pragma once

#include <antybiurokrata/libraries/objects/objects.h>

namespace core
{
	namespace reports
	{
		class summary : public Log<summary>
		{
			using Log<summary>::log;
		};

	}	 // namespace reports
}	 // namespace core