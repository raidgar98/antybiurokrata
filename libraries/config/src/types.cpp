#include <antybiurokrata/types.hpp>

template<> typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str_v& v)
{
	const core::u16str ss{v};
	return src << ss;
}

template<> typename logger::logger_piper operator<<<>(logger::logger_piper src, const core::u16str& v)
{
	*src.ss << core::get_conversion_engine().to_bytes(v);
	return src;
}

core::u16str operator"" _u16(const char* str, const size_t) { return core::get_conversion_engine().from_bytes(str); }
core::u16str operator"" _u16(const char16_t* str, const size_t s) { return core::u16str{str, s}; }

core::str operator"" _u8(const char16_t* str, const size_t) { return core::get_conversion_engine().to_bytes(str); }
core::str operator"" _u8(const char* c_str, const size_t s) { return core::str{c_str, s}; }