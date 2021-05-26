/**
 * @file safe.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains definition of thread-safe container
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <mutex>
#include <memory>
#include <functional>

namespace patterns
{
	/**
	 * @brief thread-safe container
	 * 
	 * @tparam T any type
	 */
	template<typename T> class safe
	{
		T m_value;
		std::shared_ptr<std::mutex> mtx{new std::mutex{}};

	 public:
		/**
		 * @brief Construct a new safe object
		 * 
		 * @param i_value initial value
		 */
		explicit safe(const T& i_value) : m_value{i_value} {}

		/**
		 * @brief safe acces to stored value
		 * 
		 * @param apply 
		 */
		void access(std::function<void(T&)> apply)
		{
			std::unique_lock<std::mutex> lck{*mtx};
			apply(m_value);
		}

		/** @brief proxy to access */
		void set(const T& i_value)
		{
			access([&](T& x) { x = i_value; });
		}

		/** @brief proxy to access */
		void copy(T& output)
		{
			access([&](T& x) { output = x; });
		}
	};
}	 // namespace patterns
