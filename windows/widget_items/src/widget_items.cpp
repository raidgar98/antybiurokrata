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

void account_widget_item::apply_text()
{
	if(!m_person.expired())
	{
		auto& p = (*m_person.lock().get())();
		core::u16str label{p.name()().raw + u" " + p.surname()().raw + u"[ "};
		label += static_cast<core::u16str>(p.orcid()()) + u" ]";
		setText(QString::fromStdU16String(label));
	}
}

void publication_widget_item::apply_text() 
{
	if(!m_publication.expired())
	{

		auto& p = (*m_publication.lock().get())();
		QString result = QString::fromStdString(std::to_string(p.year()));
		result.insert(0, "[ ");
		result += " ] ";
		result += QString::fromStdU16String(p.title()().raw);
		setText(result);
	}
}
