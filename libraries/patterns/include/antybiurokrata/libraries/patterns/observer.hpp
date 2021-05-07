/**
 * @file observer.hpp
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief Implementation of simple observer
 * 
 * @copyright Copyright (c) 2021
 * 
*/

/**
 * @example "observer ~ usage"
 * 
 * adding sginal option to your class:
 * ```
 * struct MySuperButton : BaseButton
 * {
 * private:
 * 	properties_t m_props{};
 *	public:
 * 	observable<
 * 		position, // what you will send on click
 * 		MySuperButton // name of this class
 * 	> on_click;
 * 
 * 	void click(position pos)
 * 	{
 * 		do_button_job();
 * 		std::cout << "hello from MySuperButton::click\n";
 * 		on_click(pos);
 * 	}
 * };
 * ```
 * 
 * registering:
 * 
 * ```
 * int main()
 * {
 * 	MySuperButton btn;
 * 	btn.on_click.register([](const postion& pos){ std::cout << "clicked on: ( " << pos.x << " ; " << pos.y << " )\n"; }, patterns::PRIORITY::LOW);
 * 	btn.click({10, 10}); // prints: "hello from MySuperButton::click\nclicked on: ( 10 ; 10 )\n"
 * 	return 0;
 * }
 * ```
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
	enum class Priority : uint8_t
	{
		HIGH	 = 3,
		MEDIUM = 2,
		LOW	 = 1
	};

	/**
	 * @brief empty struct
	 */
	struct empty_data_t
	{
	};

	/**
	 * @brief Implementation for observable 
	 * 
	 * @warning do not use this class, it's internal
	 * @tparam arg_type type of sending data with signal
	 */
	template<class arg_type = empty_data_t> class observable_impl
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
		connection register_slot(const slot_function_t& function, const Priority& p = Priority::MEDIUM)
		{
			return __signal.connect(function, static_cast<uint8_t>(p));
			// return __signal.connect(p, function);
		}

	 protected:
		/**
		 * @brief calling this function sends signal
		 * 
		 * @param arg data to send
		 */
		void invoke(const arg_type& arg) { __signal(arg); }

	 private:
		/** signal holder */
		signal<void(arg_type)> __signal;
	};

	/**
	 * @brief add this as a member to your class
	 * 
	 * @tparam arg_type type of sending data with signal
	 * @tparam __owner required to set friendship
	 */
	template<class arg_type, class __owner> class observable : protected observable_impl<arg_type>
	{
		/** required to make possible invoking by owner */
		friend __owner;

	 protected:
		/**
		 * @brief same as some_signal.invoke(arg), just fancier;
		 * 
		 * @param arg data to send
		 */
		void operator()(const arg_type& arg) { invoke(arg); }
	};

};	  // namespace patterns