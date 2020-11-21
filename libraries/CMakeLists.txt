cmake_minimum_required(VERSION 3.1)
include( "${CUSTOM_CMAKE_SCRIPTS_DIR}/get_all_directories.cmake" )
include( "${CUSTOM_CMAKE_SCRIPTS_DIR}/add_include_directories.cmake" )

set(SUBMODULES_DIR "submodules")

# first add submodules
add_subdirectory(${SUBMODULES_DIR})
SUBDIR_INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULES_DIR} )

# add all other
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(ss ${SUBDIRS})
	if(NOT ss STREQUAL ${SUBMODULES_DIR})
		add_subdirectory(${ss})
	endif()
endforeach()
