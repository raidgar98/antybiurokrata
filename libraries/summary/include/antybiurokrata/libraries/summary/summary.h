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

// STL
#include <atomic>

// Project Includes
#include <antybiurokrata/libraries/patterns/observer.hpp>
#include <antybiurokrata/libraries/patterns/safe.hpp>
#include <antybiurokrata/libraries/orm/orm.h>

namespace core
{
	namespace reports
	{

		using report_item_t		  = core::objects::publication_summary_t;
		using report_collection_t = std::vector<report_item_t>;
		using report_t				  = std::shared_ptr<report_collection_t>;

		/**
		 * @brief matches given publications and produces summary
		 */
		class summary : public Log<summary>
		{
			using Log<summary>::log;
			using publications_storage_t			  = const std::vector<objects::shared_publication_t>&;
			template<typename T> using observable = patterns::observable<T, summary>;

			// using second_publications_t = std::optional<std::ref<shared_publication_t>>;

			patterns::safe<report_t> m_report{report_t{new report_collection_t{}}};
			std::atomic<bool> is_ready{ false };

		 public:

			/**
			 * @brief Construct a new summary object, proxy to activate
			 * 
			 * @param reference reference data for comprasion, if nullptr is given, nothing cheanges
			 */
			explicit summary(publications_storage_t reference);
			explicit summary() = default;

			/**
			 * @brief Initializes all data
			 * 
			 * @param reference reference data for comprasion
			 */
			void activate(publications_storage_t reference);

			/**
			 * @brief appends comprasion output to internal storage, can take a while
			 * 
			 * @param input data to compare
			 * @param mt data source
			 */
			void process(publications_storage_t input, const objects::match_type mt);

			/**
			 * @brief called in destructor, provides pointer to return
			 */
			observable<report_t> on_done;

			/**
			 * @brief Destroy the summary object and constructs report
			 */
			~summary();

		 private:
			/**
			 * @brief actually does a job
			 * 
			 * @param input data
			 * @param mt data source
			 */
			void process_impl(publications_storage_t input, const objects::match_type mt);

			/**
			 * @brief methode for multithreading
			 * 
			 * @param item item to add
			 */
			void safely_add_report(const report_item_t& item);

			/**
			 * @brief invokes `on_done` with given object
			 * 
			 * @param obj object to send
			 */
			void invoke_on_done(report_t& obj);

			/**
			 * @brief helper function to invoke `on_done`
			 * 
			 * @param that self reference
			 */
			inline friend void invoke_on_done_helper(summary& that)
			{
				that.m_report.access([&](report_t& obj) { that.invoke_on_done(obj); });
			}
		};

	}	 // namespace reports
}	 // namespace core