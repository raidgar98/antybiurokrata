cmake_minimum_required(VERSION 3.19)
# project(logger CXX)
set(CMAKE_CXX_STANDARD 20)
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")

add_compile_definitions(BOOST_STACKTRACE_USE_BACKTRACE)

if( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo" )
	message("attachibng debug symbols")
	add_compile_options(-g3)
endif()

attach_boost(filesystem date_time stacktrace_addr2line stacktrace_noop stacktrace_basic)

create_library(logger)
target_link_libraries( logger dl ${Boost_LIBRARIES} backtrace )
