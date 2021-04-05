# project(tests CXX)
enable_testing()

include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

file(GLOB tests_sources
	"src/*.cpp"
)

message(${CMAKE_SOURCE_DIR})
# include_directories(${CMAKE_SOURCE_DIR})
attach_boost(unit_test_framework)
create_library(testbase logger types)

add_executable(tests ${tests_sources})
target_include_directories(tests PRIVATE ${Boost_INCLUDE_DIRS} ) # ${testbase_INCLUDE_DIRECTORIES} ) #${logger_INCLUDE_DIRECTORIES})

target_link_libraries( tests 
	PRIVATE

	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} 
	testbase
	demangler

	logger
)

add_test(NAME all_tests COMMAND tests)
