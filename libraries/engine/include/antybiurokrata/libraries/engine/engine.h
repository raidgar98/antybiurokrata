/**
 * @file engine.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief header for engine class
 * @version 0.1
 * @date 2021-05-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <antybiurokrata/libraries/patterns/observer.hpp>
#include <antybiurokrata/libraries/logger/logger.h>
#include <antybiurokrata/libraries/orm/orm.h>

namespace core
{
	/**
	 * @brief encapsulates all purpose of this project
	 * 
	 * @remark	this class requires lot of time to process stuff, 
	 * 			it's good idea to put evaluation to another thread
	 * 			and use signals to gather output
	 * 
	 * @remark this class internally runs 2..3 threads when processing start
	 */
	class engine : public Log<engine>
	{
	 protected:
		using Log<engine>::log;
		template<typename T> using container = std::unique_ptr<T>;

		// internal types
		using sobservable								 = patterns::sobservable<engine>;
		template<typename arg> using observable = patterns::observable<arg, engine>;
		using summary_t								 = int*;
		using error_summary_t						 = int*;

		/** @brief storage for persons extractors */
		container<orm::persons_extractor_t> m_persons_reference;
		container<orm::persons_extractor_t> m_persons_compare;

		/** @brief storage for publications extractors */
		container<orm::publications_extractor_t> m_publications_reference;
		container<orm::publications_extractor_t> m_publications_compare;

		/** @brief storage for last summary (cache a bit) */
		container<summary_t> m_last_summary;

	 public:
		/** @brief sends how many items will be processed */
		observable<size_t> on_calculated_progress;
		/** @brief sends current progess */
		observable<size_t> on_progress;

		/** @brief sends, when processing starts */
		sobservable on_start;
		/** @brief sends when finished without any problems with summary */
		observable<summary_t> on_finish;
		/** @brief sends with error summary, when something goes wrong */
		observable<error_summary_t> on_error;

		/** @brief returns last summary, if avaiable */
		const summary_t& get_last_summary() const;

		/** @brief checks is summary avaiable */
		bool is_last_summary_avaiable() const;

		/**
		 * @brief proxy to start(u16str, u16str), but get name and surname with orcid API
		 * 
		 * @param orcid orcid string
		 */
		void start(const u16str& orcid);

		/**
		 * @brief produces summary
		 * 
		 * @param name utf-8 polish name
		 * @param surname utf-8 polish surname
		 */
		void start(const u16str& name, const u16str& surname);

	 protected:
		/**
		 * @brief gets name and surname object with given orcid
		 * 
		 * @param orcid valid orcid string
		 * @param out_name output for name
		 * @param out_surname output for surname
		 * 
		 * @exception not_found_exception if cannot extract name AND surname from ORCID
		 */
		void get_name_and_surname(u16str_v orcid, u16str& out_name, u16str& out_surname) const;

		/**
		 * @brief proxy to process(u16str, u16str) but extracts name and surname from orcid and catches all errors
		 * 
		 * @param orcid 
		 */
		void process(const u16str& orcid) noexcept;

		/**
		 * @brief proxy to process_impl(u16str, u16str) and catches all exceptions
		 * 
		 * @param name valid name
		 * @param surname valid surname
		 */
		void process(const u16str& name, const u16str& surname) noexcept;

		/**
		 * @brief do all the work with creating summary as result
		 * 
		 * @param name valid name
		 * @param surname valid surname
		 * 
		 * @exception assert_exception if checks fails (lot's of checks, no sense to desctipt all of them)
		 */
		void process_impl(const u16str& name, const u16str& surname);
	};
}	 // namespace core