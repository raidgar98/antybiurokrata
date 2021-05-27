#include <antybiurokrata/libraries/summary/summary.h>

namespace core
{
	namespace reports
	{

		summary::summary(publications_storage_t reference)
		{
			m_report.access([&](report_t& obj) {
				obj->reserve(reference.size());
				for(const auto& ref: reference)
				{
					report_t::element_type::value_type to_add{};
					to_add().reference( ref );
					obj->push_back(to_add);
				}
			});
		}

		void summary::process(publications_storage_t input) { process_impl(input); }

		summary::~summary() { invoke_on_done_helper(*this); }

		void summary::safely_add_report(const report_item_t& item)
		{
			m_report.access([&item](auto& obj) { obj->emplace_back(item); });
		}

		void summary::invoke_on_done(report_t& obj) { on_done(obj); }

		void summary::process_impl(publications_storage_t input) 
		{
			using objects::publication_summary_t;
			report_t browser;
			m_report.copy(browser);
			// for( const publication_summary_t& x : *browser )
			for(size_t i = 0; i < browser->size(); ++i)
			{
				const auto& x = (*(*browser)[i]().reference()().data())();
				for(const objects::shared_publication_t& y : input)
				{
					if( x.compare(*y() ) == 0 ) // TODO: compare function is not enough; types of match has to be unique
					{
						// objects::publication_with_source_t to_add;
						m_report.access([&](report_t& obj)
						{
							(*obj)[i]().matched()().data().emplace( objects::match_type::ORCID, y );
						});
					}
				}
			}
		}

	}	 // namespace reports
}	 // namespace core