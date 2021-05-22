#include <antybiurokrata/libraries/engine/engine.h>
#include <antybiurokrata/libraries/global_adapters.hpp>

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


void engine::start(const str& orcid)
{
	setup_new_thread(get_friendly_lambda<&engine::process>(this, orcid));
}


void engine::start(const str& name, const str& surname)
{
	setup_new_thread(
		 get_friendly_lambda<&engine::process_name_and_surname>(this, name, surname, str{}));
}


engine::error_summary_t engine::prepare_error_summary()
{
	log.error() << "while processing cought unknown exception" << logger::endl;
	return nullptr;
}

engine::error_summary_t engine::prepare_error_summary(const core::exceptions::exception<str>& ex)
{
	log.error() << "while processing cought: " << ex.what() << logger::endl;
	return nullptr;
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
	catch(...)
	{
		on_error(prepare_error_summary());
	}
}

void engine::process_impl(const std::stop_token& stop_token, const str& name, const str& surname,
								  const str& orcid)
{
	auto conv = get_conversion_engine();
	const u16str w_name{conv.from_bytes(name)};
	const u16str w_surname{conv.from_bytes(surname)};
	const u16str w_orcid{conv.from_bytes(orcid)};
	const auto stop = [&]() -> bool {
		if(stop_token.stop_requested()) dassert(false, "stopped on request!"_u8);
		else [[likely]]
			return true;
		return false; /* <- dead code */
	};

	dassert{objects::detail::polish_validator{w_name}, "given name is not valid polish name"_u8};
	dassert{objects::detail::polish_validator{w_surname},
			  "given surname is not valid polish name"_u8};

	stop();


	on_finish(nullptr);
}
