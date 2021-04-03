cmake_minimum_required(VERSION 3.19)
set(PROJECT_NAME "antybiurokrata")

project(${PROJECT_NAME} LANGUAGES CXX)

# prevent build in source
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not allowed, remove CMakeFiles and CMakeCache.txt")
endif()

# update submodules
find_program(GIT_PROGRAM git)
if(NOT GIT_PROGRAM)
	message(WARNING "git not found, make sure that submodules are updated")
else()
	message("git found, updating submodules")
	execute_process(COMMAND ${GIT_PROGRAM} submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
endif()

# CMAKE paths
set(CUSTOM_CMAKE_SCRIPTS_DIR ${PROJECT_SOURCE_DIR}/cmake)
set(WINDOWS_DIR "windows")

#includes
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/add_compiler_flags.cmake")

# Qt configs
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
attach_qt()

# C++ setup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compiler_flags("-fcoroutines")

# setup threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add subdirectories
add_subdirectory(libraries)
add_subdirectory(${WINDOWS_DIR})
get_directory_property(WINDOW_LIBS DIRECTORY ${WINDOWS_DIR} DEFINITION WINDOWSES)

# create executable
add_executable( antybiurokrata main.cpp )
target_link_libraries(antybiurokrata PRIVATE Qt5::Widgets PUBLIC ${WINDOW_LIBS} )
