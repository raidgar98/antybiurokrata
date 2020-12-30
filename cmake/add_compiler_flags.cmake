MACRO(add_compiler_flags)

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${variadic})

ENDMACRO()