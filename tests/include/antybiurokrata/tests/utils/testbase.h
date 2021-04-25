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
		struct testbase_logger : public Log<testbase_logger>
		{
			using Log<testbase_logger>::get_logger;
		};

		str to_upper(const str_v& v);
	} // namespace testbase
}

namespace tests
{
	using namespace core::testbase;
	extern logger& log;
}