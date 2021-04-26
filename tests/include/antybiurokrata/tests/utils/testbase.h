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

		template<typename Any, typename ... U>
		requires std::is_constructible_v<Any, U...> and std::is_default_constructible_v<Any>
		inline bool check_serialization(U&& ... u)
		{
			std::stringstream ss;
			Any a{std::forward<U>(u)...};
			Any b{};
			ss << a;
			ss >> b;
			return a == b;
		}
	} // namespace testbase
}

namespace tests
{
	using namespace core::testbase;
	extern logger& log;
}