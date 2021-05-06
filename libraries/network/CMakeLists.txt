cmake_minimum_required(VERSION 3.19)

include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")

find_package(Drogon CONFIG REQUIRED)

attach_boost()
create_library( network logger types Drogon::Drogon )
create_library( bgpolsl_adapter logger network html_scalpel visitor )
create_library( orcid_adapter logger network visitor Drogon::Drogon )
create_library( scopus_adapter logger network visitor Drogon::Drogon )