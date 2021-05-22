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
		template<typename T> using container = std::shared_ptr<T>;

		// internal types
		using sobservable								 = patterns::sobservable<engine>;
		template<typename arg> using observable = patterns::observable<arg, engine>;
		using summary_t								 = container<orm::persons_extractor_t>;
		using error_summary_t						 = int*;
		using stop_token_t							 = std::stop_token;
		using worker_function_t						 = std::function<void(const stop_token_t&, bool&)>;

		struct process_functor_t;
		struct process_name_and_surname_functor_t;
		friend struct process_functor_t;
		friend struct process_name_and_surname_functor_t;
		struct process_functor_t
		{
			engine* that;
			str orcid;

			process_functor_t(engine* i_that, const str& i_orcid) : that{i_that}, orcid{i_orcid} {}

			virtual void operator()(const stop_token_t& token, bool& ready)
			{
				that->process(token, orcid);
				ready = true;
			}
		};
		struct process_name_and_surname_functor_t : public process_functor_t
		{
			str name;
			str surname;

			process_name_and_surname_functor_t(engine* i_that, const str& i_orcid, const str& i_name,
														  const str& i_surname) :
				 process_functor_t{i_that, i_orcid},
				 name{i_name}, surname{i_surname}
			{
			}

			virtual void operator()(const stop_token_t& token, bool& ready) override
			{
				that->process_name_and_surname(token, name, surname, orcid);
				ready = true;
			}
		};

		/** @brief storage for persons extractors */
		container<orm::persons_extractor_t> m_persons_reference;

		/** @brief storage for publications extractors */
		container<orm::publications_extractor_t> m_publications_reference;

		/** @brief storage for last summary (cache a bit) */
		container<summary_t> m_last_summary;

		/** 
		 * @brief handler for working thread 
		 * 
		 * @remark first actual worker
		 * @attention second asks: 'is it finished?' true = yes, false = no
		 */
		container<std::pair<container<std::jthread>, bool>> m_worker;

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
		 * 
		 * @remark this function automatically detaches to new thread
		 * @exception assert_exception if worker is already running
		 */
		void start(const str& orcid);

		/**
		 * @brief produces summary
		 * 
		 * @param name utf-8 polish name
		 * @param surname utf-8 polish surname
		 * 
		 * @remark this function automatically detaches to new thread
		 * @exception assert_exception if worker is already running
		 */
		void start(const str& name, const str& surname);

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
		void get_name_and_surname(const str& orcid, str& out_name, str& out_surname) const;

		/**
		 * @brief proxy to process(u16str, u16str) but extracts name and surname from orcid and catches all errors
		 * 
		 * @param orcid valid orcis string
		 */
		void process(const stop_token_t&, const str& orcid) noexcept;

		/**
		 * @brief proxy to process_impl(u16str, u16str) and catches all exceptions
		 * 
		 * @param name valid name
		 * @param surname valid surname
		 * @param orcid [optional] valid orcid
		 */
		void process_name_and_surname(const stop_token_t&, const str& name, const str& surname,
												const str& orcid = str{}) noexcept;

		/**
		 * @brief do all the work with creating summary as result
		 * 
		 * @param name valid name
		 * @param surname valid surname
		 * @param orcid [optional] valid orcid
		 * 
		 * @exception assert_exception if checks fails (lot's of checks, no sense to desctipt all of them)
		 */
		void process_impl(const stop_token_t&, const str& name, const str& surname,
								const str& orcid = str{});

		/** @brief if cannot create new thread throws assert_exception */
		void check_is_new_worker_possible() const;

		/** @brief safely constructs new thread */
		void setup_new_thread(worker_function_t fun);

		/**
		 * @brief prepares error summary, based on exception
		 * 
		 * @return error_summary_t 
		 */
		error_summary_t prepare_error_summary(const core::exceptions::exception<str>& ex);

		/**
		 * @brief prepares error summary, for unknown exception
		 * 
		 * @return error_summary_t 
		 */
		error_summary_t prepare_error_summary();
	};
}	 // namespace core