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
	class config
	{
	protected:
		virtual bool dump_config(const char *file_name) const { return false; };
		virtual bool load_config(const char *file_name) { return false; }
	};

	// config data is not deleted, only release!!!
	class configurable
	{
	protected:
		const config *_config{nullptr};

		~configurable()
		{
			_config = nullptr;
		}

		template <typename T>
		const T *get_config() const
		{
			dassert(_config);
			return dynamic_cast<const T *>(_config);
		}

	public:
		void set_config(const config *cfg, const bool)
		{
			dassert(cfg != nullptr, "cannot be nullptr");
			_config = cfg;
		}

		template <typename T>
		void set_config(const T *cfg)
		{
			if (const config *conf = reinterpret_cast<const config *>(cfg))
				set_config(conf, true);
			else
				dassert(false);
		}
	};
}