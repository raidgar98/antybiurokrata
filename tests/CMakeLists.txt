# project(tests CXX)
enable_testing()

include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

create_library( testbase logger types )

file(GLOB tests_headers
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include
	"include/${CMAKE_PROJECT_NAME}/tests/*.test.h"
)

find_package(ut REQUIRED)

set( TEST_HEADERS )
foreach( val ${tests_headers})
	set(TEST_HEADERS "${TEST_HEADERS}#include <${val}>\n")
endforeach()

set(OUTPUT_SRC_PATH ${CMAKE_CURRENT_BINARY_DIR}/tests.cpp)
configure_file(src/tests.cpp.in ${OUTPUT_SRC_PATH})

add_executable( tests ${OUTPUT_SRC_PATH})
target_link_libraries( tests 
	PRIVATE testbase
	# utlities
		logger
		types

	# Testing data
		demangler
		objects
)

target_include_directories(
	tests
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(NAME all_tests COMMAND tests)
add_custom_target(test COMMAND ./tests)