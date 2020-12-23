#pragma once

#include <boost/preprocessor.hpp>
#include <string>

// Credits: https://gist.github.com/ibab/54162246bc79279c43c7

#define X_ENUM_WITH_STRING_CONVERSION_TOSTRING_CASE(r, data, elem) \
	case data :: elem:                                                     \
		return BOOST_PP_STRINGIZE(elem);

#define X_ENUM_WITH_STRING_CONVERSION_FROMSTRING_CASE(r, data, elem) \
	if (str == BOOST_PP_STRINGIZE(elem))                             \
		return data :: elem;                                                 \
	else

// Example usage: ENUM_CLASS_WITH_STRING_CONVERSION( ANIMAL, int, UNKNOWN, (dog)(cat)(elephant) )
// Will create enum with string convertions for enum: enum class ANIMAL : int { UNKNOWN = 0, dog, cat, elephant };
#define ENUM_CLASS_WITH_STRING_CONVERSION(name, parent, invalid, enumerators) \
	enum class name : parent                                                  \
	{                                                                         \
		invalid = parent(),                                                   \
		BOOST_PP_SEQ_ENUM(enumerators)                                        \
	};                                                                        \
                                                                              \
	inline name to_##name(const std::string &str)                             \
	{                                                                         \
		BOOST_PP_SEQ_FOR_EACH(                                                \
			X_ENUM_WITH_STRING_CONVERSION_FROMSTRING_CASE,                    \
			name,                                                             \
			enumerators)                                                      \
		return name :: invalid;                                               \
	}                                                                         \
                                                                              \
	inline std::string to_string(name v)                                      \
	{                                                                         \
		switch (v)                                                            \
		{                                                                     \
			BOOST_PP_SEQ_FOR_EACH(                                            \
				X_ENUM_WITH_STRING_CONVERSION_TOSTRING_CASE,                  \
				name,                                                         \
				enumerators)                                                  \
		default:                                                              \
			return BOOST_PP_STRINGIZE(invalid);                               \
		}                                                                     \
	}                                                                         \
                                                                              \
	inline std::ostream &operator<<(std::ostream &os, const name v)           \
	{                                                                         \
		os << to_string(v);                                                   \
		return os;                                                            \
	}
