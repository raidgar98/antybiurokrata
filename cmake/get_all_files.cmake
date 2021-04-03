MACRO(GET_ALL_FILES return_list regex)
	file(GLOB_RECURSE ${return_list} regex )
ENDMACRO()

MACRO(GET_ALL_DIRS sub return_list reg)
	file(GLOB_RECURSE ${return_list} LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/${sub}/*)
	list(FILTER ${return_list} INCLUDE REGEX ${reg})
ENDMACRO()