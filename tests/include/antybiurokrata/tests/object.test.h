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

	namespace names
	{
		constexpr str_v correct_pn_01{ "żółć" };
		constexpr str_v correct_pn_02{ "jan" };
		constexpr str_v correct_pn_03{ "ZoFia" };
		constexpr str_v correct_pn_04{ "maryla" };
		constexpr str_v correct_pn_05{ "krzysztoF" };
		constexpr str_v correct_pn_06{ "ęąśćż" };

		constexpr str_v invalid_pn_01{ "" };
		constexpr str_v invalid_pn_02{ " " };
		constexpr str_v invalid_pn_03{ " aaa" };
		constexpr str_v invalid_pn_04{ "aaa " };
		constexpr str_v invalid_pn_05{ "aaa_" };
		constexpr str_v invalid_pn_06{ "aaa-" };
		constexpr str_v invalid_pn_07{ "aaa+" };
		constexpr str_v invalid_pn_08{ "a^aa" };
		constexpr str_v invalid_pn_09{ "   " };
		constexpr str_v invalid_pn_10{ "a@a" };
		constexpr str_v invalid_pn_11{ "a2a" };
		constexpr str_v invalid_pn_12{ "111" };
		constexpr str_v invalid_pn_13{ "|" };
	};
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

	const ut::suite polish_name_tests = [] {
		using namespace core::objects;
		using namespace object_tests_values::names;
		log.info() << "entering `polish_name` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;

		"case_01"_test = [] {
			const auto validate_data = [&](const str_v& x) { ut::expect(ut::eq( testbase::to_upper(x), static_cast<str>( polish_name_t{ x }() ))); };

			validate_data( correct_pn_01 );
			validate_data( correct_pn_02 );
			validate_data( correct_pn_03 );
			validate_data( correct_pn_04 );
			validate_data( correct_pn_05 );
		};

		"case_02"_test = [] {
			const auto expect_assertion = [](const str_v& x) { ut::expect( ut::throws<core::exceptions::assert_exception>( [&]{ polish_name_t{ x }; } ) ); };

			expect_assertion( invalid_pn_01 );
			expect_assertion( invalid_pn_02 );
			expect_assertion( invalid_pn_03 );
			expect_assertion( invalid_pn_04 );
			expect_assertion( invalid_pn_05 );
			expect_assertion( invalid_pn_06 );
			expect_assertion( invalid_pn_07 );
			expect_assertion( invalid_pn_08 );
			expect_assertion( invalid_pn_09 );
			expect_assertion( invalid_pn_10 );
			expect_assertion( invalid_pn_11 );
			expect_assertion( invalid_pn_12 );
			expect_assertion( invalid_pn_13 );
		};
	};

	const ut::suite person_tests = [] {
		using namespace core::objects;
		using namespace object_tests_values;
		log.info() << "entering `person_tests` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;

		"case_01"_test = [] {
			const auto check_serialization = [] (const str_v& name, const str_v& surname, const str_v& orcid) {
				person_t p_out{};
				person_t p{name, surname, orcid_t::class_t::from_string(orcid) };
				std::stringstream for_object, for_raw;
				for_object << p;
				for_object >> p_out;
				ut::expect( ut::eq( p_out, p ) );
			};

			check_serialization(names::correct_pn_01, names::correct_pn_02, orcid::correct_01);
		};
	};
}
