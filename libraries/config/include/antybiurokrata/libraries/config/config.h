/**
 * @file config.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief base classes to handle config files
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/types.hpp>

namespace core
{
	/**
	 * @brief base class for configs, practically interface
	 */
	struct config
	{
		virtual bool dump_config(const str& file_name) const { return false; }
		virtual bool load_config(const str& file_name) { return false; }
	};
}	 // namespace core