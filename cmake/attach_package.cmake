cmake_minimum_required(VERSION 3.1)

MACRO(attach_boost)

	set(variadic ${ARGN})
	find_package(Boost 1.75 REQUIRED COMPONENTS ${variadic} )

ENDMACRO()

MACRO(attach_qt)

	set(variadic ${ARGN})
	find_package(Qt5 COMPONENTS Core Widgets ${variadic} REQUIRED )

ENDMACRO()

MACRO(attach_local_libs)

	set(variadic "")
	LIST(APPEND variadic ${ARGN})
	message("variadics: ${variadic}")

	set(include_directory_list "")
	
	FOREACH(var ${variadic})
		get_property(inc GLOBAL PROPERTY ${var}_INCLUDE_DIRS)
		LIST(APPEND include_directory_list ${inc})
	ENDFOREACH()

	include_directories(${include_directory_list} )

ENDMACRO()