#pragma once

// Boost
#include <boost/test/unit_test.hpp>

// Project includes
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace exceptions
	{
		struct check_exception : public core::exceptions::exception_base<check_exception> { using core::exceptions::exception_base<check_exception>::exception_base; };
	}

	namespace testbase
	{
		struct TestLogger : public Log<TestLogger>
		{
			using Log<TestLogger>::get_logger;
		};
		using test_check = core::exceptions::require<typename core::exceptions::check_exception, true>;

		/**
		 * @brief 
		 * 
		 * 
		 * @tparam exc_t 
		 * @tparam callable_t 
		 * @param fun 
		 * @param msg 
		 * @return true 
		 * @return false 
		 */
		template<typename exc_t, typename callable_t>
		void check_exception(const callable_t fun, const char* msg = nullptr)
		{
			try
			{
				fun();
			}
			catch(const exc_t& e)
			{
				TestLogger::log.dbg(str("exception `") + get_class_name<exc_t>() + "` catched successfully.");
				if(msg)
				{
					const str exc_msg{ e.what() };
					const str expected{ msg };
					test_check{ exc_msg == msg, "missmatch in exception message: `" + expected + "` != `" + exc_msg + '`'};
					TestLogger::log.dbg("message in exception is ok");
				}
				return;
			}
			catch(...)
			{
				test_check{ false, "catched invalid exception" };
			}
			test_check{ false, "exception did not appear" };
		}

	} // namespace testbase
}

using namespace core::testbase;