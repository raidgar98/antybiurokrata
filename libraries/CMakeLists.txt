cmake_minimum_required(VERSION 3.1)
include( "${CUSTOM_CMAKE_SCRIPTS_DIR}/get_all_directories.cmake" )

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(ss ${SUBDIRS})
	add_subdirectory(${ss})
endforeach()
