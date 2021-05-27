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
	return m_last_summary;
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
	// standarize incoming data
	auto conv = get_conversion_engine();
	const objects::polish_name_t w_name{conv.from_bytes(name)};
	const objects::polish_name_t w_surname{conv.from_bytes(surname)};
	str w_orcid{orcid};
	objects::shared_person_t person{};
	if(!w_orcid.empty())
	{
		(*person())().name( w_name );
		(*person())().surname( w_surname );
		(*person())().orcid( objects::detail::detail_orcid_t::from_string( w_orcid ) );
	}else person().data().reset();


	// setup workers
	orm::persons_extractor_t scopus_visitor{};
	orm::persons_extractor_t inner_persons_extractor{};
	orm::publications_extractor_t inner_publications_extractor{inner_persons_extractor};
	std::shared_ptr<reports::summary> sum{ new reports::summary{} };

	// prepare delegates
	auto on_start_delegate = on_start.delegate_ownership();
	auto on_progress_delegate = on_progress.delegate_ownership();
	auto on_calculated_progress_delegate = on_calculated_progress.delegate_ownership();
	auto on_finish_delegate = on_finish.delegate_ownership();

	// stop token activation function
	const auto stop = [&]() {
		if(stop_token.stop_requested()) dassert(false, "stopped on request!"_u8);
	};

	{	 // local scope to delete synchronization objects, after usage

		std::mutex mtx_orcid;
		std::mutex mtx_report;
		std::condition_variable_any cv_orcid;
		std::condition_variable_any cv_report;

		const auto bgpolsl_getter = [&]() {

			// notify, that processing started
			on_start_delegate();

			// gather initial data
			auto publications_raw = ga::polsl.get_person(name, surname);

			// inform about total size of incoming data
			on_calculated_progress_delegate(publications_raw->size() * ( objects::detail::match_type_translation_unit::length - 2 /* = ( `NO_MATCH` + `POLSL` (which is reference) ) */ ));

			// extract persons
			for(auto& pub_raw: *publications_raw)
			{
				pub_raw.accept(&inner_publications_extractor);
				on_progress_delegate(1);
			}

			// setup summary engine
			auto& last_summary = this->m_last_summary;
			sum->activate(inner_publications_extractor.publications);
			global_logger << "created summary on address: " << reinterpret_cast<size_t>(sum.get()) << logger::endl;
			sum->on_done.register_slot([&](core::reports::report_t ptr){
				check_nullptr{ptr};
				last_summary = ptr;
				on_finish_delegate(ptr);
				on_progress_delegate(100);
			});
			cv_report.notify_all();
			global_logger << "cv_report notified!\n";

			// if orcid is already given return
			if(!w_orcid.empty()) return;
			for(const auto& p: inner_persons_extractor.persons)
			{
				const auto& in_p = (*p());
				if(in_p().name == w_name && in_p().surname == w_surname)
				{
					person = p;
					cv_orcid.notify_all();
					return;
				}
			}
			dassert(false, "person not found!?"_u8);
		};

		// thread order
		{
			stop();
			std::jthread th1{bgpolsl_getter};
			stop();
			
			if(w_orcid.empty())
			{
				std::unique_lock<std::mutex> lk{mtx_orcid};
				cv_orcid.wait(lk, [&]{ stop(); return !w_orcid.empty(); });
				dassert(!w_orcid.empty(), "orcid not set!"_u8);
			}

			stop();
			std::jthread th2{ core::detail::universal_getter<objects::match_type::ORCID>{ person, sum, on_progress_delegate }, std::ref(mtx_report), std::ref(cv_report) };
			stop();
			std::jthread th3{ core::detail::universal_getter<objects::match_type::SCOPUS>{ person, sum, on_progress_delegate }, std::ref(mtx_report), std::ref(cv_report) };
			stop();
		}
	}

}
