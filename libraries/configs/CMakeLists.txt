project(components CXX)
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

# SFML
attach_sfml()

create_library( config types )
create_library( paint_config config ${SFML_LIBRARIES_TO_LINK} )
