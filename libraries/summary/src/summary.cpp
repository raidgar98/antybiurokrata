#include <antybiurokrata/libraries/summary/summary.h>

namespace core
{
	namespace reports
	{
		summary::summary(publications_storage_t reference) { activate(reference); }


		void summary::activate(publications_storage_t reference)
		{
			m_report.access([&](report_t& obj) {
				obj->reserve(reference.size());
				for(const auto& ref: reference)
				{
					report_t::element_type::value_type to_add{};
					(*to_add())().reference(ref);
					obj->push_back(to_add);
				}
			});
			is_ready.store(true);
		}

		void summary::process(publications_storage_t input, const objects::match_type mt)
		{
			process_impl(input, mt);
		}

		summary::~summary() { invoke_on_done_helper(*this); }

		void summary::safely_add_report(const report_item_t& item)
		{
			m_report.access([&item](auto& obj) { obj->emplace_back(item); });
		}

		void summary::invoke_on_done(report_t& obj) { on_done(obj); }

		void summary::process_impl(publications_storage_t input, const objects::match_type mt)
		{
			dassert(is_ready.load(), "first call activate()! summry is not ready!"_u8);
			using objects::publication_summary_t;
			const objects::publication_with_source_t search{mt};
			report_t browser;
			m_report.copy(browser);
			for(size_t i = 0; i < browser->size(); ++i)
			{
				const auto& x = (*(*browser)[i]())();
				if(x.matched()().data().find(search) != x.matched()().data().end())
					continue;	// it's allready matched, no sense for processing
				const auto& ref = (*x.reference()().data())();

				for(const objects::shared_publication_t& y: input)
				{
					// TODO: compare function is not enough, but fair enough now;
					if(ref.compare(*y()) == 0)
					{
						// objects::publication_with_source_t to_add;
						m_report.access(
							 [&](report_t& obj) { (*(*obj)[i]())().matched()().data().emplace(mt, y); });
					}
				}
			}
		}

	}	 // namespace reports
}	 // namespace core