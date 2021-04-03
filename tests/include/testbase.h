#pragma once

// Boost
#include <boost/test/unit_test.hpp>

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>

namespace testbase
{
	class TestLogger : Log<TestLogger>
	{
		using Log<TestLogger>::get_logger;
	};

} // namespace testbase