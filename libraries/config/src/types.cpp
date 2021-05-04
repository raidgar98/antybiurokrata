#include <antybiurokrata/types.hpp>

template <>
typename logger::logger_piper &&operator<<<>(logger::logger_piper &&src, const core::u16str_v &v)
{
	const core::u16str ss{v.data()};
	return std::move(src) << ss;
}

template <>
typename logger::logger_piper &&operator<<<>(logger::logger_piper &&src, const core::u16str &v)
{
	const core::str gen = core::get_conversion_engine().to_bytes(v);
	return std::move(src) << gen;
}