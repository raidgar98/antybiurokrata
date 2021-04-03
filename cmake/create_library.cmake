include("${CUSTOM_CMAKE_SCRIPTS_DIR}/get_all_files.cmake")

MACRO(create_library libname)
	add_library(${libname} STATIC src/${libname}.cpp)
	target_include_directories( ${libname} PUBLIC ${BOOST_INCLUDE_DIRS} include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	set_property(GLOBAL PROPERTY ${libname}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

	if(var_length GREATER 0)
		target_link_libraries( ${libname} ${variadic} )
	endif()
ENDMACRO()

MACRO(create_qt_library libname)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	GET_ALL_FILES(header_list)
	MESSAGE("while creating Qt library `${libname}` found headers: [ ${header_list} ]")

	add_library(${libname} STATIC src/${libname}.cpp ${header_list} src/${libname}.ui)
	target_include_directories( ${libname} PUBLIC include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	target_link_libraries( ${libname} Qt5::Widgets Qt5::Core ${variadic} )

ENDMACRO()
