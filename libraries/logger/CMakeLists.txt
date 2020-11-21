cmake_minimum_required(3.16)
project(logger CXX)
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

attach_boost(filesystem date_time stacktrace_backtrace stacktrace_addr2line stacktrace_noop stacktrace_basic)

add_library(logger src/logger.cpp)
target_include_directories(logger PUBLIC include PRIVATE ${Boost_INCLUDE_DIRS} ${rang_INCLUDE_DIRS} )# ../rang/include)
target_link_libraries(logger ${Boost_LIBRARIES} )