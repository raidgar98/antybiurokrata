/**
 * @file demangler_test.cpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief theese tests checks demangler
*/

// Project includes
#include <antybiurokrata/tests/utils/testbase.h>
#include <antybiurokrata/libraries/objects/objects.h>

// using namespace core;core::
using ::logger;
using typename core::str_v;

namespace object_tests_values
{
	using namespace core;
	namespace orcid
	{
		constexpr str_v correct_01{"0000-0000-0000-0001"};
		constexpr str_v correct_02{"0000-0000-0000-0000"};
		constexpr str_v invalid_01{"0000-0000-0000-000"};
		constexpr str_v invalid_02{"0000-0000-000-0000"};
		constexpr str_v invalid_03{"0000-000-0000-0000"};
		constexpr str_v invalid_04{"000-0000-0000-0000"};
		constexpr str_v invalid_05{"0000--0000-0000"};
		constexpr str_v invalid_06{"0000-0000-0000"};
		constexpr str_v invalid_07{"000000000000000"};
		constexpr str_v invalid_08{"---"};
	}

	// NAMES

}

namespace tests
{
	using namespace core;
	using namespace boost::ut;
	namespace ut = boost::ut;

	const ut::suite orcid_tests = [] {
		using namespace core::objects;
		using namespace object_tests_values::orcid;
		log.info() << "entering `orcid_tests` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;

		const auto validation_success = [](const str_v &v) { ut::expect(ut::eq(true, orcid_t::class_t::is_valid(v))); };
		const auto validation_fail = [](const str_v &v) { ut::expect(ut::eq(false, orcid_t::class_t::is_valid(v))); };

		"case_01"_test = [&] {
			validation_success(correct_01);
			validation_success(correct_02);

			validation_fail(invalid_01);
			validation_fail(invalid_02);
			validation_fail(invalid_03);
			validation_fail(invalid_04);
			validation_fail(invalid_05);
			validation_fail(invalid_06);
			validation_fail(invalid_07);
			validation_fail(invalid_08);
		};

		"case_02"_test = [&] {
			orcid_t orcid{correct_01};
			ut::expect(ut::eq(static_cast<str>(orcid()), correct_01));
		};

		"case_03"_test = [&] {
			ut::expect(
				ut::throws<core::exceptions::assert_exception>(
					[] { orcid_t{invalid_01}; }));
		};
	};

	const ut::suite account_tests = [] {
		using namespace object_tests_values;
		using namespace core::objects;
		log.info() << "entering `orcid_tests` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;

		"case_01"_test = [&] {

		};
	};
}
