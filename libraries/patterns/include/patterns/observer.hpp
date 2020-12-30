#pragma once

//STL

// Boost
#include <boost/signals2/signal.hpp>

// Internal
#include <macros.hpp>

namespace patterns
{

	using boost::signals2::signal;
	using boost::signals2::connection;


	ENUM_CLASS_WITH_STRING_CONVERSION( Priority, uint8_t, INVALID_PRIORITY, (HIGH)(MEDIUM)(LOW) )

	// do not use this it's intenrnal implementation
	template <class arg_type>
	class observable_impl
	{
		using slot_function_t = std::function<void(arg_type)>;

	public:

		connection register_slot(const slot_function_t &function, const Priority &p = Priority::MEDIUM)
		{
			return __signal.connect(p, function);
		}

	protected:

		void invoke(const arg_type& arg)
		{
			__signal(arg);
		}

	private:
		signal<void(arg_type), Priority, uint8_t> __signal;
	};

	// add this as a member to your class ex. observable<position, my_super_button> on_click;
	// if you want to invoke, just: on_click( position{ x, y } );
	// if you want to register slot, just: my_supper_button_object.register_slot( [&](const position& pos){ do_something(pos); }, patterns::PRIORITY::HIGH );
	template <class arg_type, class __owner>
	class observable : protected observable_impl<arg_type>
	{
		friend __owner;
	protected:
		void operator()(const arg_type& arg)
		{
			invoke(arg);
		}
	};

}; // namespace patterns