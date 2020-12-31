#pragma once

// STL
#include <iostream>
#include <functional>
#include <sstream>

// submodules
#include <rang.hpp>

// Boost
#include <boost/type_index.hpp>

// STL
#include <cctype>

class logger
{
public:
	using format_function = std::function<std::ostream &(std::ostream &)>;

	template <class T>
	inline static logger _get_logger(const char * alternative = "unknown")
	{
		const std::string name = get_class_name<T>();
		for(const char c : name)
			if( std::isalnum(c) == 0 and c != '_')
				return logger(alternative);
		return logger( name );
	}

	static bool set_dump_file( const std::string& file );

	inline static format_function reset_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::reset; };
	inline static format_function debug_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::gray; };
	inline static format_function info_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::blue; };
	inline static format_function warn_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::yellow; };
	inline static format_function erro_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::red; };

	inline static format_function debug_format = [](std::ostream &os) -> std::ostream & { logger::debug_color_scheme(os); return os << "[DEBUG]"; };
	inline static format_function info_format = [](std::ostream &os) -> std::ostream & { logger::info_color_scheme(os); return os << "[INFO]"; };
	inline static format_function warn_format = [](std::ostream &os) -> std::ostream & { logger::warn_color_scheme(os); return os << "[WARNING]"; };
	inline static format_function erro_format = [](std::ostream &os) -> std::ostream & { logger::erro_color_scheme(os); return os << "[ERROR]"; };

	template <typename T>
	const logger &operator<<(const T &obj) const
	{
		std::stringstream ss;
		ss << get_preambula(3);
		ss << obj;
		print_out(ss.str(), debug_format);
		return *this;
	}

	void dbg(const std::string &) const;
	void info(const std::string &) const;
	void warn(const std::string &) const;
	void error(const std::string &) const;

	template<typename T>
	static std::string get_class_name()
	{
		return boost::typeindex::type_id<T>().pretty_name();
	}

private:

	inline static std::string dump_file;
	const std::string preambula;
	std::string get_preambula(const uint16_t depth) const;
	logger(const std::string &preambula);
	void print_out(const std::string &, const format_function &_format = logger::reset_color_scheme) const;
};

template<typename T>
inline const logger& operator<<(logger& out, const T& obj)
{
	return out.operator<<(obj);
}

template <typename T>
class Log
{
public:
	inline static logger log = logger::_get_logger<T>();

	static logger& get_logger()
	{
		return log;
	}
};