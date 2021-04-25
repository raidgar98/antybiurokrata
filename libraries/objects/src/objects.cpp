#include <antybiurokrata/libraries/objects/objects.h>

template<> 
patterns::serial::get_from_stream::get_from_stream<>
(std::istream& is, typename core::objects::detail::detail_orcid_t::storage_t& data)
{
	for(size_t i = 0; i < core::objects::detail::detail_orcid_t::words_in_orcid_num; i++)
	{
		is >> data[i];
		is.ignore(1, patterns::serial::delimiter);
	}
}

template<> 
patterns::serial::put_to_stream::put_to_stream<>
(std::ostream& os, const typename core::objects::detail::detail_orcid_t::storage_t& data)
{
	for(size_t i = 0; i < core::objects::detail::detail_orcid_t::words_in_orcid_num; i++)
		os << data[i] << patterns::serial::delimiter;
}