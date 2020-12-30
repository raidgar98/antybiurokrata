cmake_minimum_required(VERSION 3.19)
project(logger CXX)
set(CMAKE_CXX_STANDARD 20)
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

attach_boost(filesystem date_time stacktrace_addr2line stacktrace_noop stacktrace_basic)

add_library(logger src/logger.cpp)
target_include_directories(logger PUBLIC include PRIVATE ${Boost_INCLUDE_DIRS} submodules )
target_link_libraries(logger ${Boost_LIBRARIES} rang )