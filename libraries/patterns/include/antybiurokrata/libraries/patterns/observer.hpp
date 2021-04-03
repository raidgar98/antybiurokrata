/**
 * @file observer.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Implementation of simple observer
 * 
 * @copyright Copyright (c) 2021
 * 
*/
#pragma once

// Boost
#include <boost/signals2/signal.hpp>

namespace patterns
{

	using boost::signals2::connection;
	using boost::signals2::signal;

	/**
	 * @brief avaiable priorities for signals
	 */
	enum Priority : uint8_t
	{
		HIGH = 3,
		MEDIUM = 2,
		LOW = 1
	};

	/**
	 * @brief empty struct
	 */
	struct empty_data_t {};

	/**
	 * @brief Implementation for observable 
	 * 
	 * @warning do not use this class, it's internal
	 * @tparam arg_type type of sending data with signal
	 */
	template <class arg_type = empty_data_t>
	class observable_impl
	{
		using slot_function_t = std::function<void(arg_type)>;

	public:
		/**
		 * @brief registers function to call when singal is invoked
		 * 
		 * @param function function to call
		 * @param p priority (by default: MEDIUM)
		 * @return connection boost connection object
		 */
		connection register_slot(const slot_function_t &function, const Priority &p = Priority::MEDIUM)
		{
			return __signal.connect(p, function);
		}

	protected:

		/**
		 * @brief calling this function sends signal
		 * 
		 * @param arg data to send
		 */
		void invoke(const arg_type &arg)
		{
			__signal(arg);
		}

	private:
		signal<void(arg_type), Priority, uint8_t> __signal; /** signal holder */
	};

	/**
	 * @brief add this as a member to your class ex. observable<position, my_super_button> on_click;
	 * 
	 * @tparam arg_type type of sending data with signal
	 * @tparam __owner required to set friendship
	 * @example if you want to invoke, just: on_click( position{ x, y } );
	 * @example if you want to register slot, just: my_supper_button_object.register_slot( [&](const position& pos){ do_something(pos); }, patterns::PRIORITY::HIGH );
	 */
	template <class arg_type, class __owner>
	class observable : protected observable_impl<arg_type>
	{
		friend __owner; /** required to make possible invoking by owner */

	protected:

		/**
		 * @brief same as some_signal.invoke(arg), just fancier;
		 * 
		 * @param arg data to send
		 */
		void operator()(const arg_type &arg)
		{
			invoke(arg);
		}
	};

}; // namespace patterns