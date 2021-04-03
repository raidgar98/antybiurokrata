#pragma once

// STL
#include <iostream>
#include <functional>
#include <sstream>
#include <cctype>

// submodules
#include <rang.hpp>

// Boost
#include <boost/type_index.hpp>

/**
 * @brief This class provides logging system fitted for every class
 * @warning This lib shouldn't have any dependency to other libraries form this project (excluding submodules), so it's not using project aliases, like `using str = std::string`
 */
class logger
{
public:
	using format_function = std::function<std::ostream &(std::ostream &)>;

	inline static const char endl{ '\n' };

	/**
	 * @brief Returns custom logger for selected class, very simple factory
	 * 
	 * @tparam T owner class for logger
	 * @param alternative name to use if class name can't be demangled or user want another
	 * @return logger logger instance
	 */
	template <class T>
	inline static logger _get_logger(const char * alternative = "unknown")
	{
		const std::string name = get_class_name<T>();
		for(const char c : name)
			if( std::isalnum(c) == 0 and c != '_')
				return logger(alternative);
		return logger( name );
	}

	/**
	 * @brief makes avaiable to use logger as stream
	 */
	struct logger_piper
	{
		std::stringstream ss; /** used as stringinizer */ 
		const format_function& _do_reset; /** restores stream to given settings */ 

		/**
		 * @brief always resets stream to defaults and put data into stream
		 */
		~logger_piper()
		{
			std::cout << ss.str();
			_do_reset(std::cout);
		}
	};

	/**
	 * @brief sets dump file
	 * 
	 * @param file if set, logs will be outputted to stdout and file 
	 * @return true 
	 * @return false 
	 */
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

	/**
	 * @brief creates interface for stream usage
	 * 
	 * @tparam T make avaiable for any type
	 * @param obj any object
	 * @return logger_piper& which put into stdout on destruction
	 */
	template <typename T>
	logger_piper & operator<<(const T &obj) const
	{
		std::stringstream ss;
		ss << get_preambula(3);
		ss << obj;
		ss << debug_format;
		return logger_piper{ std::move(ss), reset_color_scheme };
	}

	void dbg(const std::string &) const; 
	void info(const std::string &) const;
	void warn(const std::string &) const;
	void error(const std::string &) const;

	/**
	 * @brief returns class name; basiclly proxy for boost::typeindex::type_id<>
	 * 
	 * @tparam T any class
	 * @return std::string stringinized T
	 */
	template<typename T>
	static std::string get_class_name()
	{
		return boost::typeindex::type_id<T>().pretty_name();
	}

	/**
	 * @brief prints stacktrace
	 */
	void print_stacktrace() const;

private:

	inline static std::string dump_file; /**< stores file name, where logs are dumped */
	const std::string preambula; /**< short text, which is concatenated at the beginnig of message */
	/**
	 * @brief returns preambula with additional data
	 * 
	 * @param depth how deep in stacktrace this methode should 'dive'
	 * @return std::string generated preambula
	 */
	std::string get_preambula(const uint16_t depth) const; 
	logger(const std::string &preambula);
	void print_out(const std::string &, const format_function &_format = logger::reset_color_scheme) const;
};

/**
 * @brief proxy function to inner operator
 * 
 * @tparam T any object
 * @param out self
 * @param obj any object
 * @return logger::logger_piper& self
 */
template<typename T>
inline typename logger::logger_piper& operator<<(logger& out, const T& obj)
{
	return out.operator<<(obj);
}

/**
 * @brief This is interface that can be derived by any class. Thanks to CRTP mechanism customization is automatic
 * 
 * @tparam T CRTP mechanism, put here child class
 */
template <typename T>
class Log
{
public:
	inline static logger log = logger::_get_logger<T>(); /** logger holder */
	
	/**
	 * @brief returns logger instance
	 */
	static logger& get_logger()
	{
		return log;
	}
};

/**
 * @brief operator for chaining <<
 * 
 * @tparam T any stringizable object
 * @param src self
 * @param v any object
 * @return logger::logger_piper&  returning self
 */
template<typename T> inline typename logger::logger_piper& operator<<(logger::logger_piper& src, const T& v) { src.ss << v; return src; }
