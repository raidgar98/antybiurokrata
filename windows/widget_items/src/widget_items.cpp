#include <QBrush>
#include <antybiurokrata/windows/widget_items/widget_items.h>
bool account_widget_item::operator<(const QListWidgetItem& other) const
{
	if(const auto* item = dynamic_cast<const account_widget_item*>(&other))
	{
		if(!item->m_person.expired() && !this->m_person.expired())
			return (*item->m_person.lock())().publictions()()->size()
					 < (*this->m_person.lock())().publictions()()->size();
	}

	return QListWidgetItem::operator<(other);
}

void account_widget_item::display()
{
	if(!m_person.expired())
	{
		// aliases
		auto& p = (*m_person.lock().get())();

		// creating string
		core::u16str label{p.name()().raw + u" " + p.surname()().raw + u"[ "};
		label += static_cast<core::u16str>(p.orcid()()) + u" ]";

		// style
		const QBrush brush{list_widget_colors::default_font_color};
		this->setForeground(brush);

		// setting text
		setText(QString::fromStdU16String(label));
	}
}


bool publication_widget_item::operator<(const QListWidgetItem& other) const
{
	if(const auto* item = dynamic_cast<const publication_widget_item*>(&other))
	{
		if(!this->m_publication.expired() && !item->m_publication.expired())
		{
			return (*this->m_publication.lock())().matched()().data().size()
					 < (*item->m_publication.lock())().matched()().data().size();
		}
	}

	return QListWidgetItem::operator<(other);
}


void publication_widget_item::display()
{
	if(!m_publication.expired())
	{
		// aliases
		const auto& p		  = (*m_publication.lock().get())();
		const auto& ref	  = (*p.reference()())();
		const auto& matched = p.matched()().data();

		// creating string
		QString result = QString::fromStdString(std::to_string(ref.year()));
		result.insert(0, "[ ");
		result += " ] ";
		result += QString::fromStdU16String(ref.title()().raw);

		// validate size
		const size_t color_level = matched.size();
		dassert{color_level < list_widget_colors::scale_length, "size too big!"_u8};

		// style
		const QBrush brush{list_widget_colors::match_scale_palette[color_level]};
		this->setForeground(brush);

		// setting text
		this->setText(result);
	}
}
