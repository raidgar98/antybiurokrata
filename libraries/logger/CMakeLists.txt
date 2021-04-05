cmake_minimum_required(VERSION 3.19)
project(logger CXX)
set(CMAKE_CXX_STANDARD 20)
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")

attach_boost(filesystem date_time stacktrace_addr2line stacktrace_noop stacktrace_basic)
create_library(logger)
target_link_libraries( logger dl ${Boost_LIBRARIES} )