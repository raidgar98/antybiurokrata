#pragma once

// SFML
#include <SFML/Graphics.hpp>

// Project includes
#include "../../types.hpp"
#include "config.h"

using pixel_number = floating;
using pixel_coord = sf::Vector2<floating>;

struct paint_config : public config
{
	sf::RectangleShape* end_element = nullptr;

	enum{
		FIXED,
		RANDOM
	} ship_color_type = paint_config::FIXED;
	sf::Color ship_color = sf::Color::Green;
	sf::Color ship_outline_color = sf::Color::Cyan;

	sf::Color area_color = sf::Color::White;
	sf::Color area_outline_color = sf::Color::Black;

	sf::Color cell_missed = sf::Color::Blue;
	sf::Color cell_hitted = sf::Color::Red;
	sf::Color cell_neutral = sf::Color::White;

	pixel_coord begin{ 100.0, 100.0 };

	pixel_number area_width{ 250.0 };
	pixel_number area_height{ 250.0 };

	pixel_number margin{ 5.0 };
	pixel_number padding{ 1.0 };
	pixel_number outline_width{ 0.25 };

	pixel_number ship_rows{ 20.0 };
	pixel_number ship_cols{ 20.0 };
	unumber max_ships{ 5ul };
	
	constexpr pixel_number get_cell_width() const
	{
		const pixel_number width_without_margins{ area_width - (2.0f * margin) };
		const pixel_number width_without_padding{ width_without_margins - ( ( ship_cols - 1.0f ) * padding ) };
		const pixel_number cell_width{ width_without_padding / ship_cols };
		return cell_width;
	}

	constexpr pixel_number get_cell_height() const
	{
		const pixel_number height_without_margins{ area_height - (2.0f * margin) };
		const pixel_number height_without_padding{ height_without_margins - ( ( ship_rows - 1.0f ) * padding ) };
		const pixel_number cell_height{ height_without_padding / ship_rows };
		return cell_height;
	}

	sf::Color get_ship_color() const;
};

inline std::ostream& operator<<(std::ostream& os, const pixel_coord& coord)
{
	os << "(" << coord.x << ", " << coord.y << ")";
	return os;
}