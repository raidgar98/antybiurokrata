#include "../include/paint_config.h"

// STL
#include <random>
#include <limits>

sf::Color paint_config::get_ship_color() const
{
	using color_number = sf::Uint8;
	switch( ship_color_type )
	{
		case RANDOM:
		{
			std::random_device random_engine;
			std::uniform_int_distribution< color_number > range{
				std::numeric_limits<color_number>::min(), 
				std::numeric_limits<color_number>::max()
			};
			return sf::Color( range(random_engine), range(random_engine), range(random_engine) );
		}
		default: return ship_color;
	}

}