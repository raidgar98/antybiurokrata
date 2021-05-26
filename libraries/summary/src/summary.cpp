#include <antybiurokrata/libraries/summary/summary.h>

namespace core
{
	namespace reports
	{


		summary::summary(publications_storage_t reference)
		{
			m_report.access([&](report_t& obj) {
				obj->reserve(reference.size());
				for(const auto& ref: reference) obj->push_back(ref());
			});
		}


		void summary::process(publications_storage_t input) { process_impl(input); }

		summary::~summary() { invoke_on_done_helper(*this); }

		void summary::safely_add_report(const report_item_t& item) {}

		void summary::process_impl(publications_storage_t input) {}

		void summary::invoke_on_done(report_t& obj) { on_done(obj); }


	}	 // namespace reports
}	 // namespace core