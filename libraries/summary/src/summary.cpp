#include <antybiurokrata/libraries/summary/summary.h>

namespace core
{
	namespace reports
	{

		void summary::process(publications_storage_t input) { process_impl(input); }

		summary::~summary() 
		{
			invoke_on_done_helper(*this);
		}


	}	 // namespace reports
}	 // namespace core