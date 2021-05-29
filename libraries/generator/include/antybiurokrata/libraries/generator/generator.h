/**
 * @file generator.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of required components to generate XLSX report
 * @version 0.1
 * @date 2021-05-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/summary/summary.h>

namespace core
{
	namespace reports
	{
		/**
		 * @brief creates XLSX report
		 */
		class generator : public Log<generator>
		{
			template<typename T> using observable					= patterns::observable<T, generator>;
			template<typename T> using subscription_function_t = std::function<void(T)>;
			using empty_subscription_function_t						= std::function<void()>;
			using Log<generator>::log;
			using source_data_t = reports::report_t;

			source_data_t m_data;
			str m_filename;

		 public:
			/**
			 * @brief Construct a new generator object
			 * 
			 * @param i_data data to process
			 * @param i_filename save point
			 * @param i_on_finish 
			 * @param i_on_progress 
			 */
			generator(source_data_t i_data, const str& i_filename,
						 const empty_subscription_function_t i_on_finish,
						 const subscription_function_t<size_t> i_on_progress);

			/** @brief emited when processing is finished */
			observable<void> on_finish;

			/** @brief emitted on progress */
			observable<size_t> on_progress;

			/** @brief generates report */
			void process();

			/** @brief proxy to report, handy for starting in new thread */
			void operator()() { process(); }

		 private:
			/** @brief implementation of class functionality */
			void process_impl();
		};
	}	 // namespace reports
}	 // namespace core