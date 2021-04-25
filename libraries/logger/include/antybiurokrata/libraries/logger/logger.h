/**
 * @file logger.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief complete implementation of class specific logger 
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// STL
#include <iostream>
#include <memory>
#include <functional>
#include <sstream>
#include <cctype>

// submodules
#include <rang/rang.hpp>

// Boost
#include <boost/type_index.hpp>

/**
 * @brief returns class name; basiclly proxy for boost::typeindex::type_id<>
 * 
 * @tparam T any class
 * @return std::string stringinized T
 */
template<typename T>
inline std::string get_class_name()
{
	return boost::typeindex::type_id<T>().pretty_name();
}

/**
 * @brief This class provides logging system fitted for every class
 * @warning This lib shouldn't have any dependency to other libraries form this project (excluding submodules), so it's not using project aliases, like `using str = std::string`
 */
class logger
{
public:
	using format_function = std::function<std::ostream &(std::ostream &)>;

	inline static const char endl{ '\n' };

	/** @brief describes  */
	enum class log_level : uint16_t
	{
		NONE	= 0,			/** @brief disables logging */
		ERROR	= 1,			/** @brief prints only error messags */
		WARN	= 2,			/** @brief prints errors + warnings */
		INFO	= 3,			/** @brief prints errors + warnings + info */

		/** @brief prints all  */
		DEBUG	= std::numeric_limits<uint16_t>::max()
	};

	inline static log_level level{ log_level::DEBUG }; // by default log all

	template<log_level lvl>
	constexpr static bool log_on_current_log_level()
	{
		const uint16_t current = static_cast<uint16_t>(logger::level);
		const uint16_t incoming = static_cast<uint16_t>(lvl);
		return incoming <= current;
	}

	template<log_level lvl>
	constexpr static void set_current_log_level() { logger::level = lvl; }
	constexpr static void set_default_log_level() { set_current_log_level<log_level::DEBUG>(); }
	template<log_level lvl>
	struct switch_log_level_keeper
	{
		explicit switch_log_level_keeper() { set_current_log_level<lvl>(); }
		~switch_log_level_keeper() { set_default_log_level(); }
	};

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
		using ss_t = std::stringstream;

		/** restores stream to given settings */
		format_function _do_reset;  

		/** used as stringinizer */ 
		std::shared_ptr<ss_t> ss;

		/** specifies whether steream will be flushed tio standard output */
		bool will_be_printed;

		logger_piper(ss_t&& xss, const bool print_out, const format_function& ff) : _do_reset{ff}, will_be_printed{print_out}
		{
			ss = std::shared_ptr<ss_t>{ new ss_t{ std::move(xss) }, [&](ss_t* ptr)
			{
				if(ptr)
				{
					if(will_be_printed) std::cout << ptr->str();
					_do_reset(std::cout);
					delete ptr;
				}
			} };
		}
	};

	/**
	 * @brief sets dump file
	 * 
	 * @param file if set, logs will be outputted to stdout and file 
	 * @return true if opening file goes well
	 * @return false if opening file failed
	 */
	static bool set_dump_file( const std::string& file );

	inline static format_function reset_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::reset;	}; 
	inline static format_function debug_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::gray;		};
	inline static format_function info_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::blue;		};
	inline static format_function warn_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::yellow;	};
	inline static format_function erro_color_scheme = [](std::ostream &os) -> std::ostream & { return os << rang::bg::reset << rang::fg::red;		};

	inline static format_function debug_format = [](std::ostream &os) -> std::ostream & { logger::debug_color_scheme(os); return os << "[DEBUG]"; 	};
	inline static format_function info_format = [](std::ostream &os) -> std::ostream & { logger::info_color_scheme(os); return os << "[INFO]"; 		};
	inline static format_function warn_format = [](std::ostream &os) -> std::ostream & { logger::warn_color_scheme(os); return os << "[WARNING]"; 	};
	inline static format_function erro_format = [](std::ostream &os) -> std::ostream & { logger::erro_color_scheme(os); return os << "[ERROR]"; 	};

	/**
	 * @brief creates interface for stream usage
	 * 
	 * @return logger_piper& struct that takes care about flush collected data
	*/
	logger_piper start_stream() const
	{
		return _config_logger_piper();
	}

	void dbg(const std::string &) const; 
	void info(const std::string &) const;
	void warn(const std::string &) const;
	void error(const std::string &) const;

	logger_piper dbg() const; 
	logger_piper info() const;
	logger_piper warn() const;
	logger_piper error() const;

	/**
	 * @brief prints stacktrace
	 */
	void print_stacktrace() const;

private:

	/** stores file name, where logs are dumped */
	inline static std::string dump_file; 

	/** short text, which is concatenated at the beginnig of message */
	const std::string preambula; 
	/**
	 * @brief returns preambula with additional data
	 * 
	 * @param depth how deep in stacktrace this methode should 'dive'
	 * @return std::string generated preambula
	 */
	std::string get_preambula(const uint16_t depth) const; 
	logger(const std::string &preambula);
	void print_out(const std::string &, const format_function &_format = logger::reset_color_scheme) const;

	logger_piper _config_logger_piper(const bool print_out = true, const format_function& fun = debug_format) const
	{
		std::stringstream ss;
		fun(ss);
		ss << get_preambula(4);
		return logger_piper{ std::move(ss), print_out, reset_color_scheme };
	}
};

/**
 * @brief This is interface that can be derived by any class. Thanks to CRTP mechanism customization is automatic
 * 
 * @tparam T CRTP mechanism, put here child class
 */
template <typename T>
class Log
{
public:
	/** logger holder */
	inline static logger log = logger::_get_logger<T>(); 
	
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
template<typename T> inline typename logger::logger_piper&& operator<<(logger::logger_piper&& src, const T& v) { *src.ss << v; return std::move(src); }

/**
 * @brief proxy function to inner operator
 * 
 * @tparam T any object
 * @param out self
 * @param obj any object
 * @return logger::logger_piper& self
 */
template<typename T>
inline typename logger::logger_piper operator<<(logger& out, const T& obj)
{
	return out.start_stream() << obj;
}
