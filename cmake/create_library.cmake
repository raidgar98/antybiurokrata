include("${CUSTOM_CMAKE_SCRIPTS_DIR}/get_all_files.cmake")

MACRO(create_library libname)

	add_library(${libname} STATIC src/${libname}.cpp)
	target_include_directories( ${libname} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${BOOST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} )
	message("for ${libname} added include dir: ${CMAKE_CURRENT_SOURCE_DIR}/include")
	
	set(${libname}_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)

	
	if(var_length GREATER 0)
		target_link_libraries( ${libname} ${variadic} )
		foreach( pack ${variadic} )
			message("for ${libname} adding pack: ${pack} with includes: ${${pack}_INCLUDE_DIRECTORIES}")
			if(pack STREQUAL "Drogon::Drogon") 
				continue() 
			endif()
			include_directories(${${pack}_INCLUDE_DIRECTORIES})
		endforeach()
	endif()
ENDMACRO()

MACRO(create_qt_library libname)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	GET_ALL_FILES(header_list *.h(pp)? *.ui )
	MESSAGE("while creating Qt library `${libname}` found headers: [ ${header_list} ]")

	add_library(${libname} STATIC src/${libname}.cpp ${header_list} src/${libname}.ui)
	target_include_directories( ${libname} PUBLIC include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)
	target_link_libraries( ${libname} Qt5::Widgets Qt5::Core ${variadic} )

	foreach( pack ${variadic} )
		message("for ${libname} adding pack: ${pack} with includes: ${${pack}_INCLUDE_DIRECTORIES}")
		if(pack STREQUAL "Drogon::Drogon") 
			continue() 
		endif()
		include_directories(${${pack}_INCLUDE_DIRECTORIES})
	endforeach()

ENDMACRO()

MACRO(create_qt_library_no_ui libname)
    set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	GET_ALL_FILES(header_list *.h(pp)? *.ui )
	MESSAGE("while creating Qt library `${libname}` found headers: [ ${header_list} ]")

	add_library(${libname} STATIC src/${libname}.cpp ${header_list})
	target_include_directories( ${libname} PUBLIC include )

	set(variadic ${ARGN})
	list(LENGTH variadic var_length)
	target_link_libraries( ${libname} Qt5::Widgets Qt5::Core ${variadic} )

	foreach( pack ${variadic} )
		message("for ${libname} adding pack: ${pack} with includes: ${${pack}_INCLUDE_DIRECTORIES}")
		if(pack STREQUAL "Drogon::Drogon")
			continue()
		endif()
		include_directories(${${pack}_INCLUDE_DIRECTORIES})
	endforeach()

ENDMACRO()
