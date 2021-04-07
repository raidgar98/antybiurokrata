/**
 * @file demangler_test.cpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief theese tests checks demangler
*/

// Project includes
#include <antybiurokrata/tests/utils/testbase.h>
#include <antybiurokrata/libraries/demangler/demangler.h>

// using namespace core;core::
using ::logger;
using core::demangler;

template <typename T>
inline const char *cast(const T ptr) { return reinterpret_cast<const char *>(ptr); }

namespace demangler_tests_values
{
	constexpr char msg_01[] = "aaa";
	const char *msg_02 = cast(u8"ąąą");
	constexpr char msg_03[] = "&#261;&#261;&#261;";
	constexpr char msg_04[] = "abba";
	constexpr char msg_05[] = "abbą";
	constexpr char msg_06[] = "abb&#261;";
}

namespace tests
{
	using namespace boost::ut;
	const suite demangler_tests = [] {
		using namespace demangler_tests_values;
		log << "entering `demangler_tests` suite" << logger::endl;
		"case_01"_test = [] {
			demangler dmg{msg_01};
			expect(
				 throws<typename core::exceptions::assert_exception>(
					  [&]() { dmg.get(); }));
		};
	};
}

/*
BOOST_AUTO_TEST_SUITE(demangler_tests)

BOOST_AUTO_TEST_CASE(test_case_01)
{
	demangler dmg{msg_01};
	check_exception<
		typename core::exceptions::assert_exception>([&]() {
		dmg.get();
	}, "data is not processed yet, run `process` first");
}

BOOST_AUTO_TEST_CASE(test_case_02)
{
	const auto dmg = demangler{msg_01}();
	test_check{dmg.get() == msg_01, "text should't change"};
}

BOOST_AUTO_TEST_CASE(test_case_03)
{
	const auto dmg = demangler{msg_02}();
	TestLogger::get_logger() << "test_case_03: " << dmg.get() << "\n";
	test_check{dmg.get() == msg_01, "text should change to msg_01"};
}

BOOST_AUTO_TEST_CASE(test_case_04)
{
	const auto dmg = demangler{msg_02}.process<core::demangler::conv_t::HTML>();
	test_check{dmg.get() == msg_03, "text should change to msg_03"};
}

BOOST_AUTO_TEST_CASE(test_case_05)
{
	const auto dmg = demangler{msg_05}();
	test_check{dmg.get() == msg_04, "text should change to msg_04"};
}

BOOST_AUTO_TEST_CASE(test_case_06)
{
	const auto dmg = demangler{msg_05}.process<core::demangler::conv_t::HTML>();
	test_check{dmg.get() == msg_06, "text should change to msg_06"};
}

BOOST_AUTO_TEST_SUITE_END()

*/