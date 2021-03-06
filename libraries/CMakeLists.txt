cmake_minimum_required(VERSION 3.1)
include( "${CUSTOM_CMAKE_SCRIPTS_DIR}/get_all_directories.cmake" )
include( "${CUSTOM_CMAKE_SCRIPTS_DIR}/add_include_directories.cmake" )

set(LOGGER_DIR "logger")
set(CONFIG_DIR "config")
set(XLSX_DIR "QtXlsxWriter")

# first add submodule
set( CXX_BACKUP ${CMAKE_CXX_FLAGS} )	# backup arguments
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w" )	# temporarly disable all warnings (for submodule)
add_compile_definitions(XLSX_NO_LIB)
add_subdirectory(${XLSX_DIR})
set(xlsx_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/QtXlsxWriter/src" CACHE INTERNAL "")
set( CMAKE_CXX_FLAGS "${CXX_BACKUP} -Wall -Wextra -Werror" )	# enable all possible warnings and treat them as errors

# next logger
add_subdirectory(${LOGGER_DIR})

# and add configs
add_subdirectory(${CONFIG_DIR})

# add all other
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(ss ${SUBDIRS})
	if(
		NOT ss STREQUAL ${XLSX_DIR} AND 
		NOT ss STREQUAL ${LOGGER_DIR} AND
		NOT ss STREQUAL ${CONFIG_DIR} 
	)
		message("adding lib: ${ss}")
		add_subdirectory(${ss})
	endif()
endforeach()
