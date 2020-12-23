cmake_minimum_required(VERSION 3.1)

project(antybiurokrata LANGUAGES CXX)

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

#includes
include("$CUSTOM_CMAKE_SCRIPTS_DIR/attach_package.cmake")

# Qt configs
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++ setup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# setup threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(libraries)
add_subdirectory(windows)

add_executable(antybiurokrata
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
)

target_link_libraries(antybiurokrata PRIVATE Qt5::Widgets)
