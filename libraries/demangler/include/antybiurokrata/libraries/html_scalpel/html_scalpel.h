/**
 * @file html_scalpel.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of html_scalpel function
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

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