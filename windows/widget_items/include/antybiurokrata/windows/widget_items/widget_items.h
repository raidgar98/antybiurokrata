#pragma once

#include <antybiurokrata/libraries/objects/objects.h>
#include <QListWidgetItem>

using namespace core;

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

	virtual bool operator<(const QListWidgetItem& other) const override;

 private:
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
	void apply_text();
};