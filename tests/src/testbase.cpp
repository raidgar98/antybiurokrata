#include <antybiurokrata/tests/utils/testbase.h>

core::str core::testbase::to_upper(const core::str_v& v)
{
	auto eng = core::get_conversion_engine();
	u16str u16result, buffer{eng.from_bytes(v.data())};
	u16result.reserve(buffer.size());
	for(const u16char_t c: buffer) u16result += static_cast<u16char_t>(std::toupper(static_cast<wchar_t>(c), core::plPL()));
	return eng.to_bytes(u16result);
}

namespace tests
{
	logger& log = core::testbase::testbase_logger::get_logger();
}