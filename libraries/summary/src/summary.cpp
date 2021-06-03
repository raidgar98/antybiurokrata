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
			// dassert(is_ready.load(), "first call activate()! summry is not ready!"_u8);
			while(!is_ready.load()) { std::this_thread::yield(); };
			using objects::publication_summary_t;
			const objects::publication_with_source_t search{mt};
			report_t browser;
			m_report.copy(browser);

			for(auto& item : *browser)
			{
				auto& report_item = (*item())();
				auto& matches = report_item.matched()().data();
				
				if(matches.find(search) != matches.end())
					continue;	// it's allready matched, no sense for processing
				const auto& ref = (*report_item.reference()().data())();

				for(const objects::shared_publication_t& y: input)
				{
					/**
					 * @todo `compare` is minimum to make program works, but much much better results can be achieved 
					 * by replacing this primitive function with on of theese algorithms
					 * 
					 * (1) https://en.wikipedia.org/wiki/Damerau%E2%80%93Levenshtein_distance
					 * (2) https://en.wikipedia.org/wiki/Levenshtein_distance
					 * 
					 * Damerau–Levenshtein (1) distance probably will better fit here
					 * 
					 * This change is required to increase matching score, because often in titles, misspells occurs
					 */
					if(ref.compare(*y()) == 0)
					{
						m_report.access(
							 [&](report_t&) { matches.emplace(mt, y); });
					}
				}
			}
		}

	}	 // namespace reports
}	 // namespace core