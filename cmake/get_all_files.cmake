MACRO(GET_ALL_FILES return_list)
    FILE(GLOB_RECURSE ${return_list} *.h(pp)?)
ENDMACRO()
