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

#include <antybiurokrata/libraries/objects/objects.h>
#include <QListWidgetItem>

using namespace core;

/**
 * @brief widget item with referation to source of its data
 */
struct account_widget_item : public QListWidgetItem
{

	using w_person_t	  = std::shared_ptr<objects::person_t>;
	using weak_person_t = std::weak_ptr<objects::person_t>;
	weak_person_t m_person;

	account_widget_item(const w_person_t& i_person, QListWidget* parent = nullptr) :
		 QListWidgetItem{parent}, m_person{i_person}
	{
		apply_text();
	};

	/**
	 * @brief thanks to this operator, neighbours can be sorted by amount of collaboration with searched person
	 */
	virtual bool operator<(const QListWidgetItem& other) const override;

 private:

	/**
	 * @brief set text on widdget based on given person
	 */
	void apply_text();
};

struct publication_widget_item : public QListWidgetItem
{
	using w_pub_t	  = std::shared_ptr<objects::publication_t>;
	using weak_pub_t = std::weak_ptr<objects::publication_t>;
	weak_pub_t m_publication;

	publication_widget_item(const w_pub_t& i_publication, QListWidget* parent = nullptr) :
		 QListWidgetItem{parent}, m_publication{i_publication}
	{
		apply_text();
	};

 private:

	/**
	 * @brief set text on widdget based on given publication
	 */
	void apply_text();
};