#pragma once

// Project includes
#include "../../types.hpp"

// If you need more singletons of same type, change second parameter of template
template<typename T, typename _ = void>
class singleton
{
private:
	inline static std::unique_ptr<T> __item;

public:

	static void set(T* item)
	{
		__item.reset(item);
	}

	static T& get()
	{
		require(__item.get() != nullptr);
		return *__item;
	}

	using value_type = T;
};