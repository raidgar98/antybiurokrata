#pragma once

// Boost
#include <boost/ut.hpp>
// import boost.ut;

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace testbase
	{
		struct TestLogger : public Log<TestLogger>
		{
			using Log<TestLogger>::get_logger;
		};
	} // namespace testbase
}

namespace tests
{
	using namespace core::testbase;
	logger& log = core::testbase::TestLogger::get_logger();
}