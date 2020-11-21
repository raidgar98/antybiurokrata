#include "logger.h"

// STL
#include <fstream>

// Boost
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>

logger::logger(const std::string &preambula)
	: preambula{preambula} {}

std::string logger::get_preambula(const uint16_t depth) const
{
	std::stringstream ss;
	ss << "[" << boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time()) << "]"; // time
	ss << "[" << std::string(preambula) << "]";																					// logger name
	
	const std::vector<boost::stacktrace::frame> frames{boost::stacktrace::stacktrace().as_vector()};

	if (depth < frames.size()) // info about log position
	{
		const boost::stacktrace::frame &fr{frames[depth]};
		if(fr.source_line() > 0) ss << "[" << boost::filesystem::path(fr.source_file()).filename().c_str() << ":" << fr.source_line() << "]" << "[" << fr.name() << "]";
	}
	return ss.str() + " ";
}

void logger::print_out(const std::string &msg, const format_function &_format) const
{
	if (not logger::dump_file.empty())
	{
		std::ofstream file{ logger::dump_file, std::ios::app };
		if (file.good())
		{
			_format(file);
			file << msg << std::endl;
			logger::reset_color_scheme( file );
			file << '\0';
		}
		file.close();
	}

	_format(std::cout);
	std::cout << msg << std::endl;
	logger::reset_color_scheme(std::cout);
	std::cout << "";
}

void logger::dbg(const std::string & msg) const
{
	print_out( get_preambula(2) + msg, debug_format );
}

void logger::info(const std::string & msg) const
{
	print_out( get_preambula(2) + msg, info_format );
}

void logger::warn(const std::string & msg) const
{
	print_out( get_preambula(2) + msg, warn_format );
}

void logger::error(const std::string & msg) const
{
	print_out( get_preambula(2) + msg, erro_format );
}

bool logger::set_dump_file( const std::string& file )
{
	logger logg = logger::_get_logger<logger>("logger");

	if(boost::filesystem::exists(boost::filesystem::path(file)))
		logg.warn("File " + file + " will be deleted." );

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