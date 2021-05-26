/**
 * @file summary.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contatins declaration of mechanism to compare results
 * @version 0.1
 * @date 2021-05-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <antybiurokrata/libraries/orm/orm.h>
#include <antybiurokrata/libraries/patterns/observer.hpp>

namespace core
{
	#if false
	namespace reports
	{

		using report_item_t = core::objects::publication_summary_t;
		using report_collection_t = std::vector<report_item_t>;
		using report_t = std::shared_ptr<report_collection_t>;

		/**
		 * @brief matches given publications and produces summary
		 */
		class summary : public Log<summary>
		{
			using Log<summary>::log;
			using publication_storage_t = core::orm::publication_storage_t;
			using publications_storage_t = const std::vector<publication_storage_t>&;
			template<typename T> using observable = patterns::observable<T, summary>;

			// using second_publications_t = std::optional<std::ref<publication_storage_t>>;

			publications_storage_t m_reference;
			std::map<

		 public:

			/**
			 * @brief Construct a new summary object
			 * 
			 * @tparam other_owner owner type of incoming signal
			 * @param reference reference data for comprasion
			 * @param on_incoming_data reference to signal, that adds new data to analyze
			 */
			template<typename other_owner> // <- optionaly can be removed by linking to engine, but it's without logical sense in dependency hierarchy
			summary(publications_storage_t reference, patterns::observable<publication_storage_t, other_owner>& on_incoming_data ) : m_reference{ reference }
			{
				on_incoming_data.register_slot( [&](publication_storage_t incoming_data){ this->process(incoming_data); });
			}

			/**
			 * @brief appends comprasion output to internal storage, can take a while
			 * 
			 * @param input data to compare
			 */
			void process(publications_storage_t input);

			/**
			 * @brief called in destructor, provides pointer to return
			 */
			observable<report_t> on_done;

			/**
			 * @brief Destroy the summary object and constructs report
			 */
			~summary();

		private:
			
			void process_impl(publications_storage_t input);

			void safely_add_report(const report_item_t& item);

			void 
			// 
		};

	}	 // namespace reports
	#endif
}	 // namespace core