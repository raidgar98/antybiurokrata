#pragma once

#include <antybiurokrata/types.hpp>

namespace core
{
	/**
     * @brief this function can be used to extract words from html (removing tags)
     * 
     * @param input_html 
     * @param output 
     */
	void html_scalpel(const str_v& input_html, std::vector<u16str>& output);
}	 // namespace core