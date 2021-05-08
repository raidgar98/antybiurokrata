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
using typename core::str_v;

namespace demangler_tests_values
{
	constexpr str_v msg_01{"aaa"};
	constexpr str_v msg_02{"ąąą"};

	// HTML
	constexpr str_v msg_03{"&#0261&#0261&#0261"};
	constexpr str_v msg_04{"abba"};
	constexpr str_v msg_05{"abbą"};
	constexpr str_v msg_06{"abb&#0261"};
	constexpr str_v msg_07{"&#0261&#0261&xxx;"};
	constexpr str_v msg_08{"&#0261&&&&&#0261"};
	constexpr str_v msg_09{"&#0261&#&#&#&#0261"};
	constexpr str_v msg_10{"&&&&&&&#0261&#&#&#&#0261"};
	constexpr str_v msg_11{"ąą&xxx;"};
	constexpr str_v msg_12{"ą&&&&ą"};
	constexpr str_v msg_13{"ą&#&#&#ą"};
	constexpr str_v msg_14{"&&&&&&ą&#&#&#ą"};
	constexpr str_v msg_15{"#261;"};

	// URL
	constexpr str_v msg_16{"%C4%85"};					// ą
	constexpr str_v msg_17{"%C4%85%C4%85%C4%85"};	// ąąą
	constexpr str_v msg_18{"C4%85"};
	constexpr str_v msg_19{"aaaaaaC4%85"};
	constexpr str_v msg_20{"aaaaaa%C4%85"};	// aaaaaaą
	constexpr str_v msg_21{"%%%%%%%C4%%85"};
	constexpr str_v msg_22{"%%%%%%%C%85"};
	constexpr str_v msg_23{"ą"};
	constexpr str_v msg_24{"aaaaaaą"};
	constexpr str_v msg_25{"%4%85"};
	constexpr str_v msg_26{"%C%85"};
	constexpr str_v msg_27{"%C4%5"};
	constexpr str_v msg_28{"%C4%8"};
	constexpr str_v msg_29{"%C%5"};
	constexpr str_v msg_30{"%C44%85"};
	constexpr str_v msg_31{"%C4%851"};
	constexpr str_v msg_32{"ą1"};
}	 // namespace demangler_tests_values

namespace tests
{
	using namespace boost::ut;
	namespace ut = boost::ut;

	const ut::suite demangler_tests = [] {
		using namespace demangler_tests_values;
		log.info() << "entering `demangler_tests` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;

		"case_01"_test = [] {
			demangler dmg{msg_01};
			ut::expect(ut::throws<typename core::exceptions::assert_exception<core::str> >([&]() { dmg.get(); }));
		};

		"case_02"_test = [] {
			const auto dmg = demangler<>{msg_01}();
			ut::expect(ut::eq(dmg.get(), msg_01));
		};

		"case_03"_test = [] {
			const auto dmg = demangler<>{msg_02}();
			ut::expect(ut::eq(dmg.get(), msg_01));
		};

		"case_04"_test = [] {
			const auto dmg = demangler<>{msg_02}.process<core::conv_t::HTML>();
			ut::expect(ut::eq(dmg.get(), msg_03));
		};

		"case_05"_test = [] {
			const auto dmg = demangler<>{msg_05}();
			ut::expect(ut::eq(dmg.get(), msg_04));
		};

		"case_06"_test = [] {
			const auto dmg = demangler<>{msg_05}.process<core::conv_t::HTML>();
			ut::expect(ut::eq(dmg.get(), msg_06));
		};

		"case_07"_test = [] {
			logger::set_current_log_level<logger::log_level::DEBUG>();
			core::str result{msg_03};
			demangler<>::mangle<core::conv_t::HTML>(result);
			ut::expect(ut::eq(result, msg_02));
		};

		"case_08"_test = [] {
			auto check = [](const str_v& in, const str_v& exp) -> void {
				core::str result{in};
				demangler<>::mangle<core::conv_t::HTML>(result);
				ut::expect(ut::eq(exp, result));
			};

			check(msg_07, msg_11);
			check(msg_08, msg_12);
			check(msg_09, msg_13);
			check(msg_10, msg_14);
			check(msg_15, msg_15);
		};

		"case_09"_test = [] {
			auto check = [](const str_v& in, const str_v& exp) -> void {
				core::str result{in};
				demangler<>::mangle<core::conv_t::URL>(result);
				ut::expect(ut::eq(exp, result));
			};

			check(msg_16, msg_23);
			check(msg_17, msg_02);
			check(msg_18, msg_18);
			check(msg_19, msg_19);
			check(msg_20, msg_24);
			check(msg_21, msg_21);
			check(msg_22, msg_22);
			check(msg_25, msg_25);
			check(msg_26, msg_26);
			check(msg_27, msg_27);
			check(msg_28, msg_28);
			check(msg_29, msg_29);
			check(msg_30, msg_30);
			check(msg_31, msg_32);
		};
	};
}	 // namespace tests
