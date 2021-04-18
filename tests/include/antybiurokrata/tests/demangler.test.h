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

template <typename T>
inline const char *cast(const T ptr) { return reinterpret_cast<const char *>(ptr); }

namespace demangler_tests_values
{
	constexpr str_v msg_01{"aaa"};
	constexpr str_v msg_02{"ąąą"};
	constexpr str_v msg_03{"&#261;&#261;&#261;"};
	constexpr str_v msg_04{"abba"};
	constexpr str_v msg_05{"abbą"};
	constexpr str_v msg_06{"abb&#261;"};
	constexpr str_v msg_07{"&#261;&#261;&xxx;"};
	constexpr str_v msg_08{"&#261;&&&&&#261;"};
	constexpr str_v msg_09{"&#261;&#&#&#&#261;"};
	constexpr str_v msg_10{"&&&&&&&#261;&#&#&#&#261;"};
	constexpr str_v msg_11{"ąą&xxx;"};
	constexpr str_v msg_12{"ą&&&&ą"};
	constexpr str_v msg_13{"ą&#&#&#ą"};
	constexpr str_v msg_14{"&&&&&&ą&#&#&#ą"};
}

namespace tests
{
	using namespace boost::ut;
	namespace ut = boost::ut;

	const ut::suite demangler_tests = [] {
		using namespace demangler_tests_values;
		log << "entering `demangler_tests` suite" << logger::endl;
		"case_01"_test = [] {
			demangler dmg{msg_01};
			ut::expect(
				ut::throws<typename core::exceptions::assert_exception>(
					[&]() { dmg.get(); }));
		};

		"case_02"_test = [] {
			const auto dmg = demangler{msg_01}();
			ut::expect(ut::eq(dmg.get(), msg_01));
		};

		"cast_03"_test = [] {
			const auto dmg = demangler{msg_02}();
			ut::expect(ut::eq(dmg.get(), msg_01));
		};

		"cast_04"_test = [] {
			const auto dmg = demangler{msg_02}.process<core::demangler::conv_t::HTML>();
			ut::expect(ut::eq(dmg.get(), msg_03));
		};

		"cast_05"_test = [] {
			const auto dmg = demangler{msg_05}();
			ut::expect(ut::eq(dmg.get(), msg_04));
		};

		"cast_06"_test = [] {
			const auto dmg = demangler{msg_05}.process<core::demangler::conv_t::HTML>();
			ut::expect(ut::eq(dmg.get(), msg_06));
		};

		"cast_07"_test = [] {
			core::str result{ msg_03 };
			demangler::mangle<core::demangler::conv_t::HTML>(result);
			ut::expect( ut::eq( result, msg_02 ) );
		};

		"cast_08"_test = []{
			auto check = [](const str_v& in, const str_v& exp) -> void
			{
				core::str result{ in };
				demangler::mangle<core::demangler::conv_t::HTML>(result);
				ut::expect( ut::eq(exp, result) );
			};

			check( msg_07, msg_11 );
			check( msg_08, msg_12 );
			check( msg_09, msg_13 );
			check( msg_10, msg_14 );
		};
	};
}
