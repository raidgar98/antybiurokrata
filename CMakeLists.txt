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
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=5")
endif()

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

# doxygen
option(DOXYGEN "determines to generate docs or not" ON)
IF(${DOXYGEN})

	# Preparing tree driectory
	set(DOXYFILE_PATH_SOURCE "${CMAKE_SOURCE_DIR}/gen/docs/Doxyfile.in")
	set(DOXYFILE_PATH_DEST "${CMAKE_BINARY_DIR}/docs")
	file(MAKE_DIRECTORY ${DOXYFILE_PATH_DEST})
	file(COPY ${DOXYFILE_PATH_SOURCE} DESTINATION ${DOXYFILE_PATH_DEST})
	set(DOXYFILE_PATH_DEST "${DOXYFILE_PATH_DEST}/Doxyfile")
	file(RENAME ${DOXYFILE_PATH_DEST}.in ${DOXYFILE_PATH_DEST})

	# Setting up sources
	file(APPEND ${DOXYFILE_PATH_DEST} "\nINPUT = ${CMAKE_SOURCE_DIR}/libraries ${CMAKE_SOURCE_DIR}/windows \n")
	file(APPEND ${DOXYFILE_PATH_DEST} "OUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/docs \n")
	file(APPEND ${DOXYFILE_PATH_DEST} "EXCLUDE_PATTERNS = ${CMAKE_SOURCE_DIR}/libraries/QtXlsxWriter \n")
	
	# Theme
	set(THEME_SUBMODULE_PATH ${CMAKE_SOURCE_DIR}/gen/docs/doxygen-dark-theme)
	# set(THEME_SUBMODULE_CSS "\"${THEME_SUBMODULE_PATH}/custom.css\" \"${THEME_SUBMODULE_PATH}/custom_dark_theme.css\"")

	# file(APPEND ${DOXYFILE_PATH_DEST} "HTML_EXTRA_STYLESHEET = ${THEME_SUBMODULE_CSS} \n")
	# file(APPEND ${DOXYFILE_PATH_DEST} "HTML_HEADER = \"${THEME_SUBMODULE_PATH}/html_header.html\"\n")
	# file(APPEND ${DOXYFILE_PATH_DEST} "HTML_FOOTER = \"${THEME_SUBMODULE_PATH}/html_footer.html\"\n")
	file(APPEND ${DOXYFILE_PATH_DEST} "PROJECT_LOGO = \"${CMAKE_SOURCE_DIR}/gen/docs/image.png\" \n")


	# Adding target
	find_program(DOXYGEN_BINARY doxygen HINTS /usr/bin)
	# To run: `make docs`
	add_custom_target(docs COMMAND ${DOXYGEN_BINARY} ${DOXYFILE_PATH_DEST} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
ENDIF()

option(FORMAT "determins to create make format that performes format all files !!! IN SOURCE DIR !!!" ON)
IF(${FORMAT})

	# get all files to format
	file(GLOB_RECURSE FILES_TO_FORMAT LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR} *.hpp *.cpp *.h *.test.h *.cpp.in)

	# filter out direcotries
	set(FILES_FOR_CLANG_FORMAT)
	foreach(x ${FILES_TO_FORMAT})
		if(x MATCHES "antybiurokrata/gen/.*" OR x MATCHES "tests/src/tests.cpp.in" OR x MATCHES "libraries/QtXlsxWriter/.*")
			continue() 
		endif()
		list(APPEND FILES_FOR_CLANG_FORMAT "${x}")
	endforeach()

	# locate program
	find_program(CLANG_FORMAT_BINARY clang-format HINTS /usr/bin)

	# To run: `make format` in build directory
	add_custom_target(format COMMAND ${CLANG_FORMAT_BINARY} -style=file -i ${FILES_FOR_CLANG_FORMAT} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
ENDIF()


# add tests
add_subdirectory(tests)
