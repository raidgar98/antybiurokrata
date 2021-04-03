cmake_minimum_required(VERSION 3.1)

MACRO(attach_boost)

	set(variadic ${ARGN})
	find_package(Boost 1.75 REQUIRED COMPONENTS ${variadic} )

ENDMACRO()

MACRO(attach_qt)

	set(variadic ${ARGN})
	find_package(Qt5 COMPONENTS Core Widgets ${variadic} REQUIRED )

ENDMACRO()
