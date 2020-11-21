cmake_minimum_required(VERSION 3.1)
project(macros CXX)

add_library( macros src/macros.cpp )
target_include_directories( macros PUBLIC include )