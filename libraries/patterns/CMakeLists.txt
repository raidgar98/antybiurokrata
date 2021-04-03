include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

create_library( visitor logger )
create_library( singleton logger )
create_library( observer )

create_library( generator )
create_library( recursive_type_collection )
create_library( functor recursive_type_collection )
