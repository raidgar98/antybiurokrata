/**
 * @file visitor.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Implementation of (invited) visitor
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// STL
#include <cassert>
#include <memory>

// Boost
#include <boost/core/demangle.hpp>

// Project includes
#include <logger/logger.h>

namespace patterns
{

	/**
	 * @brief derive over this to make class visitor
	 * 
	 * @tparam T CRTP - pass here child class type
	 */
	template <typename T>
	class visits : private Log<visits<T>>
	{
		using Log<visits<T>>::get_logger;

	public:
		/**
		 * @brief this is called when this design pattern is used
		 * 
		 * @param ptr pointer to any type
		 * @return true if visit goes well
		 * @return false stop signal or somethiong goes wrong
		 */
		virtual bool visit(T *ptr)
		{
			get_logger().warn("Empty visit, by: `" + boost::typeindex::type_id<T>().pretty_name() + "`.");
			return true;
		}
	};

	/**
	 * @brief derive over this to make class visitable by visitor
	 * 
	 * @tparam T CRTP - pass here child class type
	 */
	template <class T>
	class Visitable : private Log<Visitable<T>>
	{
		using Log<Visitable<T>>::get_logger;

	public:
		/**
		 * @brief proxy to accept
		 * 
		 * @param v visitor type
		 * @return true if visit goes well
		 * @return false stop signal or somethiong goes wrong
		 */
		bool operator()(visits<T>* v) { return accept(v); }

		/**
		 * @brief claim visitor
		 * 
		 * @param v visitor type
		 * @return true if visit goes well
		 * @return false stop signal or somethiong goes wrong
		 */
		virtual bool accept(visits<T> *v)
		{
			get_logger().info("Accepted: `" + boost::typeindex::type_id<T>().pretty_name() + "`.");
			return v->visit(dynamic_cast<T *>(this));
		}
	};

} // namespace patterns