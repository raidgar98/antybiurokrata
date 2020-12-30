MACRO(create_library libname)
	add_library(${libname} STATIC src/${libname}.cpp)
	target_include_directories( ${libname} PUBLIC ${BOOST_INCLUDE_DIRS} include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	if(var_length GREATER 0)
		target_link_libraries( ${libname} ${variadic} )
	endif()
ENDMACRO()

MACRO(create_qt_library libname)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	add_library(${libname} STATIC src/${libname}.cpp include/${libname}/${libname}.h src/${libname}.ui)
	target_include_directories( ${libname} PUBLIC include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	target_link_libraries( ${libname} Qt5::Widgets Qt5::Core ${variadic} )

ENDMACRO()
