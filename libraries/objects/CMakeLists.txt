cmake_minimum_required(VERSION 3.19)

include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

attach_boost()
create_library( objects logger types serializer )