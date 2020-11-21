#pragma once

// Boost
#include <boost/test/unit_test.hpp>

// Project includes
#include <logger/logger.h>

#define CHECK_ASSERTION_WITH_MESSAGE( Expression, Exception, RequiredMessage ) BOOST_REQUIRE_EXCEPTION( Expression, Exception, [](const Exception& e){ return (RequiredMessage == "" ? true : e.what() == RequiredMessage); } )
#define CHECK_ASSERTION( Expression, Exception ) CHECK_ASSERTION_WITH_MESSAGE( Expression, Exception, "" )

namespace testbase
{

	class TestLogger : Log<TestLogger>
	{
		using TestLogger::log;
	};

} // namespace testbase

#define lout testbase::TestLogger::log