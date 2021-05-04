/**
 * @file demangler_test.cpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief theese tests checks string splitter
*/

// Project includes
#include <antybiurokrata/tests/utils/testbase.h>
#include <antybiurokrata/types.hpp>

// using namespace core;core::
using ::logger;
using typename core::str_v;

namespace string_splitter_tests_values
{
	namespace wide_string_names
	{
		using str_v = core::u16str_v;
		constexpr str_v words_01{ u"w1,w2,w3" };
		constexpr str_v words_02{ u"w1 w2 w3" };
		constexpr str_v words_03{ u"w1w2w3" };
		constexpr str_v words_04{ u"" };
		constexpr str_v words_05{ u"w" };
		constexpr str_v words_06{ u"Barglik Jerzy 0000-0002-1994-3266 912576 054 RM" };
	}

	namespace simple_string_names
	{
		using str_v = core::str_v;
		constexpr str_v words_01{ "w1,w2,w3" };
		constexpr str_v words_02{ "w1 w2 w3" };
		constexpr str_v words_03{ "w1w2w3" };
	}
}

namespace tests
{
	using namespace core;
	using namespace boost::ut;
	namespace ut = boost::ut;

	const ut::suite string_splitter_tests = [] {
		log.info() << "entering `string_splitter_tests` suite" << logger::endl;
		logger::switch_log_level_keeper<logger::log_level::NONE> _;
		using namespace core::string_utils;

		"case_01"_test = [&] {
			using namespace string_splitter_tests_values::simple_string_names;
			split_words<str_v> ss{words_01, ','};
			auto it = ss.begin();
			ut::expect(ut::eq( *it, str_v{"w1"} ));
			it++;
			ut::expect(ut::eq( *it, str_v{"w2"} ));
			it++;
			ut::expect(ut::eq( *it, str_v{"w3"} ));
			it++;
			ut::expect(it == ss.end());
			ut::expect(ut::throws<core::exceptions::assert_exception>([&it]{it++;}));
		};

		"case_02"_test = [&]{
			using namespace string_splitter_tests_values::simple_string_names;
			str res{};
			for(auto v : split_words<str_v>{words_01, ','}) res += v;
			ut::expect(ut::eq(res,words_03));
		};

		"case_03"_test = [&]{
			using namespace string_splitter_tests_values::simple_string_names;
			str res{};
			for(auto v : split_words<str_v>{words_02, ' '}) res += v;
			ut::expect(ut::eq(res,words_03));
		};

		"case_04"_test = [&] {
			using namespace string_splitter_tests_values::wide_string_names;
			split_words<u16str_v> ss{words_01, u','};
			auto it = ss.begin();
			ut::expect( *it == u16str_v{u"w1"} );
			it++;
			ut::expect( *it == u16str_v{u"w2"} );
			it++;
			ut::expect( *it == u16str_v{u"w3"} );
			it++;
			ut::expect(it == ss.end());
			ut::expect(ut::throws<core::exceptions::assert_exception>([&it]{it++;}));
		};

		"case_05"_test = [&]{
			using namespace string_splitter_tests_values::wide_string_names;
			u16str res{};
			for(auto v : split_words<u16str_v>{words_01, u','}) res += v;
			ut::expect(res == words_03);
		};

		"case_06"_test = [&]{
			using namespace string_splitter_tests_values::wide_string_names;
			u16str res{};
			for(auto v : split_words<u16str_v>{words_02, u' '}) res += v;
			ut::expect(res == words_03);
		};

		"case_07"_test = [&]{
			using namespace string_splitter_tests_values::wide_string_names;
			u16str res{};
			for(auto v : split_words<u16str_v>{words_04, u' '}) res += v;
			ut::expect(res == u"");
		};

		"case_08"_test = [&]{
			using namespace string_splitter_tests_values::wide_string_names;
			u16str res{};
			for(auto v : split_words<u16str_v>{words_05, u' '}) res += v;
			ut::expect(res == words_05);
		};

		"case_09"_test = [&]{
			using namespace string_splitter_tests_values::wide_string_names;
		// constexpr str_v words_06{ u"Barglik Jerzy 0000-0002-1994-3266 912576 054 RM" };
			split_words<u16str_v> splitter{words_06, u' '};
			auto it = splitter.begin();
			const auto check_next = [&it](const u16str_v& valid) { ut::expect(*it ==  valid); it++; };
			check_next(u"Barglik");
			check_next(u"Jerzy");
			check_next(u"0000-0002-1994-3266");
			check_next(u"912576");
			check_next(u"054");
			check_next(u"RM");
		};
	};
}