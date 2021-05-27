#include <antybiurokrata/libraries/engine/engine.h>
#include <antybiurokrata/libraries/global_adapters.hpp>

#include <condition_variable>
#include <shared_mutex>

namespace ga = core::network::global_adapters;
using namespace core;

void engine::get_name_and_surname(const str& orcid, str& out_name, str& out_surname) const
{
	try
	{
		ga::orcid.get_name_and_surname(orcid, out_name, out_surname);
	}
	catch(...)
	{
		throw core::exceptions::not_found_exception<str>{"given orcid not found"_u8};
	}
}

void engine::check_is_new_worker_possible() const
{
	// if std::jthread is created, then second [bool] has to be set to true
	dassert{(m_worker.get() == nullptr)
					|| (m_worker.get() != nullptr
						 && ((m_worker->first.get() == nullptr)
							  || (m_worker->first.get() != nullptr && m_worker->second == true))),
			  "cannot start new worker, current one is not set!"_u8};
}

void engine::setup_new_thread(worker_function_t fun)
{
	check_is_new_worker_possible();

	if(m_worker.get() == nullptr) m_worker.reset(new std::pair<container<std::jthread>, bool>{});
	m_worker->first.reset(new std::jthread{fun, std::ref<bool>(m_worker->second)});
}


const engine::summary_t& engine::get_last_summary() const
{
	dassert(is_last_summary_avaiable(),
			  "last summary is not avaiable! first check before calling"_u8);
	return *m_last_summary;
}


bool engine::is_last_summary_avaiable() const { return m_last_summary.get() != nullptr; }


void engine::start(const str& orcid) { setup_new_thread(process_functor_t{this, orcid}); }


void engine::start(const str& name, const str& surname)
{
	setup_new_thread(process_name_and_surname_functor_t{this, str{}, name, surname});
}


engine::error_summary_t engine::prepare_error_summary() const
{
	return std::make_shared<core::exceptions::error_report>(
		 "while processing cought unknown exception");
}

engine::error_summary_t engine::prepare_error_summary(
	 const core::exceptions::exception<str>& ex) const
{
	return std::make_shared<core::exceptions::error_report>(ex);
}

engine::error_summary_t engine::prepare_error_summary(
	 const core::exceptions::exception<u16str>& ex) const
{
	return std::make_shared<core::exceptions::error_report>(ex);
}

void engine::process(const std::stop_token& stop_token, const str& orcid) noexcept
{
	try
	{
		dassert(core::objects::orcid_t::value_t::is_valid_orcid_string(orcid),
				  "given string is not valid orcid!"_u8);
		str name{}, surname{};
		get_name_and_surname(orcid, name, surname);
		if(stop_token.stop_requested()) return;
		process_impl(stop_token, name, surname, orcid);
	}
	catch(const core::exceptions::exception<str>& e)
	{
		on_error(prepare_error_summary(e));
	}
	catch(const core::exceptions::exception<u16str>& e)
	{
		on_error(prepare_error_summary(e));
	}
	catch(...)
	{
		on_error(prepare_error_summary());
	}
}

void engine::process_name_and_surname(const std::stop_token& stop_token, const str& name,
												  const str& surname, const str& orcid) noexcept
{
	try
	{
		process_impl(stop_token, name, surname, orcid);
	}
	catch(const core::exceptions::exception<str>& e)
	{
		on_error(prepare_error_summary(e));
	}
	catch(const core::exceptions::exception<u16str>& e)
	{
		on_error(prepare_error_summary(e));
	}
	catch(...)
	{
		on_error(prepare_error_summary());
	}
}

void engine::process_impl(const std::stop_token& stop_token, const str& name, const str& surname,
								  const str& orcid)
{
	auto conv = get_conversion_engine();
	const objects::polish_name_t w_name{conv.from_bytes(name)};
	const objects::polish_name_t w_surname{conv.from_bytes(surname)};
	log.info() << patterns::serial::pretty_print{w_name} << logger::endl;
	log.info() << w_name().raw << logger::endl;
	log.info() << patterns::serial::pretty_print{w_surname} << logger::endl;
	log.info() << w_surname().raw << logger::endl;
	orm::persons_extractor_t orcid_visitor{};
	orm::persons_extractor_t scopus_visitor{};
	str w_orcid{orcid};


	const auto stop = [&]() {
		if(stop_token.stop_requested()) dassert(false, "stopped on request!"_u8);
	};

	{	 // local scope to delete synchronization objects, after usage
		std::shared_mutex mtx_orcid;
		std::shared_mutex mtx_report;
		std::condition_variable_any cv_orcid;
		std::condition_variable_any cv_report;
		auto& inner_persons_extractor		  = m_persons_reference;
		auto& inner_publications_extractor = m_publications_reference;
		std::unique_ptr<reports::summary> sum;

		const auto bgpolsl_getter = [&]() {
			inner_persons_extractor.reset(new orm::persons_extractor_t{});
			inner_publications_extractor.reset(
				 new orm::publications_extractor_t{*inner_persons_extractor});

			// on_start();

			auto publications_raw = ga::polsl.get_person(name, surname);

			// on_calculated_progress(persons_data->size());

			for(auto& pub_raw: *publications_raw)
			{
				// on_progress(1)
				pub_raw.accept(&(*inner_publications_extractor));
			}

			sum.reset(new reports::summary{inner_publications_extractor->publications});
			sum->on_done.register_slot([](core::reports::report_t ptr){
				check_nullptr{ptr};
				global_logger << logger::endl << logger::endl << logger::endl;
				for(const auto& x : *ptr) global_logger.info() << patterns::serial::pretty_print{ x } << logger::endl;
				global_logger << logger::endl << logger::endl << logger::endl;
			});
			cv_report.notify_all();

			if(!w_orcid.empty()) return;
			for(const auto& p: inner_persons_extractor->persons)
			{
				const auto& person = (*p());
				if(person().name == w_name && person().surname == w_surname)
				{
					w_orcid = objects::orcid_t::value_t::to_string(person().orcid()());
					cv_orcid.notify_all();
					return;
				}
			}
			dassert(false, "person not found!?"_u8);
		};

		const auto orcid_getter = [&]() {
			std::shared_lock<std::shared_mutex> lk{mtx_orcid};
			if(w_orcid.empty()) cv_orcid.wait(lk);	  // wait just once
			dassert(!w_orcid.empty(), "orcid not set!"_u8);
			network::orcid_adapter::result_t orcid_ret = ga::orcid.get_person(w_orcid);

			orm::persons_extractor_t::shallow_copy_persons(*inner_persons_extractor, orcid_visitor);
			orm::publications_extractor_t pub_visitor{orcid_visitor};
			for(auto& x: *orcid_ret) x.accept(&pub_visitor);

			global_logger << logger::endl << logger::endl << logger::endl;
			for(const auto& pub : pub_visitor.publications)
				global_logger.info() << patterns::serial::pretty_print{ pub } << logger::endl;
			global_logger << logger::endl << logger::endl << logger::endl;

			{
				std::shared_lock<std::shared_mutex> lk{mtx_report};
				cv_report.wait(lk, [&sum] { return static_cast<bool>(sum); });
			}

			// generate report
			check_nullptr{sum};
			sum->process(pub_visitor.publications);
		};

		const auto scopus_getter = [&]() {
			// wait for lock
			std::shared_lock<std::shared_mutex> lk{mtx_orcid};
			if(w_orcid.empty()) cv_orcid.wait(lk);	  // wait just once
			dassert(!w_orcid.empty(), "orcid not set!"_u8);

			// acquire data
			network::scopus_adapter::result_t scopus_ret = ga::scopus.get_person(w_orcid);

			// process data
			orm::persons_extractor_t::shallow_copy_persons(*inner_persons_extractor, scopus_visitor);
			orm::publications_extractor_t pub_visitor{scopus_visitor};
			for(auto& x: *scopus_ret) x.accept(&pub_visitor);

			// wait for report generator
			{
				std::shared_lock<std::shared_mutex> lk{mtx_report};
				cv_report.wait(lk, [&sum] { return static_cast<bool>(sum); });
			}

			// generate report
			check_nullptr{sum};
			sum->process(pub_visitor.publications);
		};

		{
			stop();
			on_start();
			stop();
			std::jthread th1{bgpolsl_getter};
			stop();
			std::jthread th2{orcid_getter};
			stop();
			std::jthread th3{scopus_getter};
			stop();
		}
	}

	on_finish(m_persons_reference);
}
