#pragma once

#include "../../types.hpp"

class config
{
protected:
	virtual bool dump_config(const char* file_name) const { return false; };
	virtual bool load_config(const char* file_name) { return false; }
};

// config data is not deleted, only release!!!
class configurable
{
protected:
	const config* _config{ nullptr };

	~configurable()
	{
		_config = nullptr;
	}

	template<typename T>
	const T* get_config() const 
	{
		require( _config );
		return dynamic_cast<const T*>(_config);
	}

public:

	void set_config(const config* cfg, const bool)
	{
		require( cfg );
		_config = cfg;
	}

	template<typename T>
	void set_config(const T* cfg)
	{
		if( const config* conf = reinterpret_cast<const config*>(cfg) ) set_config(conf, true);
		else require(false);
	}
};
