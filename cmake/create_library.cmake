MACRO(create_library libname)
	add_library(${libname} STATIC src/${libname}.cpp)
	target_include_directories( ${libname} PUBLIC ${BOOST_INCLUDE_DIRS} include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	if(var_length GREATER 0)
		target_link_libraries( ${libname} ${variadic} )
	endif()
ENDMACRO()