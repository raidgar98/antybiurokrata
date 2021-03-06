#include <antybiurokrata/libraries/logger/logger.h>

// STL
#include <fstream>

// Boost
#include <boost/stacktrace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>

namespace ___dummmy
{
	struct global_logger
	{
	};
};	  // namespace ___dummmy
logger global_logger{logger::_get_logger<___dummmy::global_logger>()};

void logger::log_with_global_logger(const std::string& msg) { global_logger.dbg() << msg; }

logger::logger(const std::string& preambula) : preambula{preambula} {}

std::string logger::get_preambula(const uint16_t depth) const
{
	std::stringstream ss;
	ss << "["
		<< boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time())
		<< "]";												 // time
	ss << "[" << std::string(preambula) << "]";	 // logger name

	const std::vector<boost::stacktrace::frame> frames{boost::stacktrace::stacktrace().as_vector()};

	if(depth < frames.size())	 // info about log position
	{
		const boost::stacktrace::frame& fr{frames[depth]};
		if(fr.source_line() > 0)
			ss << "[" << boost::filesystem::path(fr.source_file()).filename().c_str() << ":"
				<< fr.source_line() << "]"
				<< "[" << fr.name() << "]";
	}
	return ss.str() + " ";
}

void logger::print_out(const std::string& msg, const format_function& _format)
{
	if(!logger::dump_file.empty())
	{
		std::ofstream file{logger::dump_file, std::ios::app};
		if(file.good())
		{
			_format(file);
			file << msg;
			logger::reset_color_scheme(file);
			file << '\0';
		}
		file.close();
	}

	_format(std::cout);
	std::cout << msg;
	// logger::reset_color_scheme(std::clog);
	std::cout << "";
	std::cout.flush();
}

void logger::print_stacktrace() const
{
	// std::stringstream ss{};
	// ss << std::endl << std::endl << boost::stacktrace::stacktrace();
	// print_out( get_preambula(2) + ss.str(), debug_format );
	dbg() << logger::endl << boost::stacktrace::stacktrace();
}

void logger::dbg(const std::string& msg) const
{
	if(!log_on_current_log_level<logger::log_level::DEBUG>()) return;
	print_out(get_preambula(2) + msg + logger::endl, debug_format);
}

void logger::info(const std::string& msg) const
{
	if(!log_on_current_log_level<logger::log_level::INFO>()) return;
	print_out(get_preambula(2) + msg + logger::endl, info_format);
}

void logger::warn(const std::string& msg) const
{
	if(!log_on_current_log_level<logger::log_level::WARN>()) return;
	print_out(get_preambula(2) + msg + logger::endl, warn_format);
}

void logger::error(const std::string& msg) const
{
	if(!log_on_current_log_level<logger::log_level::ERROR>()) return;
	print_out(get_preambula(2) + msg + logger::endl, erro_format);
}

logger::logger_piper logger::dbg() const
{
	return _config_logger_piper(log_on_current_log_level<logger::log_level::DEBUG>());
}

logger::logger_piper logger::info() const
{
	return _config_logger_piper(log_on_current_log_level<logger::log_level::INFO>(),
										 logger::info_format);
}

logger::logger_piper logger::warn() const
{
	return _config_logger_piper(log_on_current_log_level<logger::log_level::WARN>(),
										 logger::warn_format);
}

logger::logger_piper logger::error() const
{
	return _config_logger_piper(log_on_current_log_level<logger::log_level::ERROR>(),
										 logger::erro_format);
}

bool logger::set_dump_file(const std::string& file)
{
	logger logg = logger::_get_logger<logger>("logger");

	if(boost::filesystem::exists(boost::filesystem::path(file)))
		logg.warn("File " + file + " will be deleted.");

	std::ofstream f(file);
	if(f.good()) f << "";
	else
	{
		logg.error("Failed to create " + file);
		return false;
	}
	f.close();

	logger::dump_file = file;
	return true;
}