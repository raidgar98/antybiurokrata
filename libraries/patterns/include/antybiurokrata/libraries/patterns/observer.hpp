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
 * 	btn.on_click.register([](const postion& pos){ std::cout << "clicked on: ( " << pos.x << " ; " << pos.y << " )\n"; });
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

	/** @brief empty struct, alternative to void */
	struct empty_data_t
	{
	};

	template<typename arg_type> struct call_ownership_delegator{};

	/**
	 * @brief holds signal object and provides basic methode of calling
	 * 
	 * @tparam arg_type argument for signal to send
	 */
	template<class arg_type> struct observable_impl_data_holder
	{
		using slot_function_t = std::function<void(arg_type)>;
		friend struct call_ownership_delegator<arg_type>;

	 protected:
		/**
		 * @brief calling this function sends signal
		 * 
		 * @param arg data to send
		 */
		void invoke(const arg_type& arg) { this->__signal(arg); }

		/** @brief signal holder */
		signal<void(arg_type)> __signal;
	};

	/** @brief specialization for void parameter */
	template<> struct observable_impl_data_holder<void>
	{
		using slot_function_t = std::function<void()>;
		friend struct call_ownership_delegator<void>;

	 protected:
		/** @brief calling this function sends signal */
		void invoke() { this->__signal(); }

		/** @brief signal holder */
		signal<void()> __signal;
	};

	/**
	 * @brief Implementation for observable 
	 * 
	 * @warning do not use this class, it's internal
	 * @tparam arg_type type of sending data with signal
	 */
	template<class arg_type = empty_data_t>
	class observable_impl : public observable_impl_data_holder<arg_type>
	{
		using slot_function_t = typename observable_impl_data_holder<arg_type>::slot_function_t;

	 public:
		/**
		 * @brief registers function to call when singal is invoked
		 * 
		 * @param function function to call
		 * @return connection boost connection object
		 */
		connection register_slot(const slot_function_t& function)
		{
			return this->__signal.connect(function);
		}

	protected:
		auto delegate_ownership() { return call_ownership_delegator<arg_type>{ *this }; }

	};

	/**
	 * @brief this is class allows to delegate privillage of calling to other objects/functions
	 * 
	 * @tparam arg_type type of sending data
	 */
	template<typename arg_type> requires( !std::is_same_v<arg_type, void> ) struct call_ownership_delegator<arg_type>
	{
		friend class observable_impl<arg_type>;
		void operator()(const arg_type& arg) { invoke.invoke(arg); }

		call_ownership_delegator(const call_ownership_delegator&) = default;
		call_ownership_delegator(call_ownership_delegator&&) = default;

	private:
		/** @brief constructor is restricted for friend */
		explicit call_ownership_delegator(observable_impl<arg_type>& that) : invoke{ that } {}
		observable_impl<arg_type>& invoke;
	};

	/* @brief specialization for void type */
	template<> struct call_ownership_delegator<void>
	{
		friend class observable_impl<void>;
		void operator()() { invoke.invoke(); }

		call_ownership_delegator(const call_ownership_delegator&) = default;
		call_ownership_delegator(call_ownership_delegator&&) = default;

	private:
		/** @brief constructor is restricted for friend */
		explicit call_ownership_delegator(observable_impl<void>& that) : invoke{ that } {}
		observable_impl<void>& invoke;
	};

	/**
	 * @brief add this as a member to your class
	 * 
	 * @tparam arg_type type of sending data with signal
	 * @tparam __owner required to set friendship
	 */
	template<class arg_type, class __owner> class observable : public observable_impl<arg_type>
	{
		/** @brief required to make possible invoking by owner */
		friend __owner;

	 protected:
		/**
		 * @brief same as some_signal.invoke(arg), just fancier;
		 * 
		 * @param arg data to send
		 */
		void operator()(const arg_type& arg) { observable_impl_data_holder<arg_type>::invoke(arg); }
	};

	/**
	 * @brief specialization of class above, for argument-free calls
	 * 
	 * @tparam __owner required to set friendship
	 */
	template<class __owner> class observable<void, __owner> : public observable_impl<void>
	{
		/** @brief required to make possible invoking by owner */
		friend __owner;

	 protected:
		/** @brief same as some_signal.invoke(), just fancier */
		void operator()() { invoke(); }
	};

	/** @brief [S]hort observable */
	template<typename __owner> using sobservable = observable<void, __owner>;

};	  // namespace patterns