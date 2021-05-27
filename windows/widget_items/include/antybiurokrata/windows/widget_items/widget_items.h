/**
 * @file widget_items.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declations of more helpful list widget items
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// #include <antybiurokrata/libraries/objects/objects.h>
#include <antybiurokrata/libraries/summary/summary.h>
#include <QListWidgetItem>
#include <QColor>

using namespace core;

namespace list_widget_colors
{
	constexpr uint8_t opacity{225};
	constexpr QColor default_font_color{28, 28, 28, 255};
	constexpr QColor green_color{78, 155, 71, opacity};
	constexpr QColor orange_color{207, 173, 32, opacity};
	constexpr QColor red_color{183, 61, 68, opacity};
	constexpr QColor match_scale_palette[] = {red_color, orange_color, green_color};
	constexpr size_t scale_length{sizeof(match_scale_palette) / sizeof(QColor)};
}	 // namespace list_widget_colors

/**
 * @brief widget item with referation to source of its data
 */
struct account_widget_item : public QListWidgetItem
{
	using w_person_t	  = objects::shared_person_t;
	using weak_person_t = typename w_person_t::value_t::value_type::weak_type;
	weak_person_t m_person;

	account_widget_item(const w_person_t& i_person, QListWidget* parent = nullptr) :
		 QListWidgetItem{parent}, m_person{i_person().data()}
	{
		display();
	};

	/**
	 * @brief thanks to this operator, neighbours can be sorted by amount of collaboration with searched person
	*/
	virtual bool operator<(const QListWidgetItem& other) const override;

 private:
	/**
	 * @brief set text on widdget based on given person
	*/
	void display();
};


struct publication_widget_item : public QListWidgetItem
{
	using incoming_t = core::reports::report_item_t;
	using weak_pub_t = typename incoming_t::value_t::value_type::weak_type;
	weak_pub_t m_publication;

	publication_widget_item(const incoming_t& i_publication, QListWidget* parent = nullptr) :
		 QListWidgetItem{parent}, m_publication{i_publication().data()}
	{
		display();
	};

	virtual bool operator<(const QListWidgetItem& other) const override;

 private:
	/**
	 * @brief set text on widdget based on given publication
	 */
	void display();
};