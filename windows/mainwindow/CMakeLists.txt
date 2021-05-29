cmake_minimum_required(VERSION 3.1)

include("${CUSTOM_CMAKE_SCRIPTS_DIR}/attach_package.cmake")
include("${CUSTOM_CMAKE_SCRIPTS_DIR}/create_library.cmake")

attach_qt()
create_qt_library( mainwindow info_dialog widget_items types bgpolsl_adapter orcid_adapter scopus_adapter orm engine generator )
