#!/usr/bin/python3

import os
from sys import argv
from os.path import join as join_path

PROJECT_NAME = "antybiurokrata"
LIBRARY_DIR_NAME = "libraries"
DESTINATION_DIR = f"../../{LIBRARY_DIR_NAME}"

print("checking aruments...")
assert len(argv) == 2, 'not enough arguments, pass window name. Ex: ./create_new_window.py super_dooooooper_window_2'
LIB_NAME : str = argv[1]
LIB_PATH = join_path(DESTINATION_DIR, LIB_NAME)

INPUT_DIRECTORY = join_path( os.curdir, 'example_library.in' )

print("checking cwd...")
assert os.path.exists(INPUT_DIRECTORY), f'no "{INPUT_DIRECTORY}" found'

print("creating paths...")
HEADER_FILE_PATH = join_path( INPUT_DIRECTORY, 'lib.h.in' )
SOURCE_FILE_PATH = join_path( INPUT_DIRECTORY, 'lib.cpp.in' )
CMAKE_FILE_PATH = join_path( INPUT_DIRECTORY, 'lib.cmake.in' )

PRE_INCLUDE_DIRECTORY = join_path( LIB_PATH, "include" )
INCLUDE_PROJ_DIR = join_path( PRE_INCLUDE_DIRECTORY, PROJECT_NAME )
INCLUDE_LIB_DIR = join_path( INCLUDE_PROJ_DIR, LIBRARY_DIR_NAME )
INCLUDE_DIRECTORY = join_path( INCLUDE_LIB_DIR, LIB_NAME )
SRC_DIRECTORY = join_path( LIB_PATH, "src" )

swap_dict = {
	"ExLib": LIB_NAME
}

print("creating directory structure...")
from os import mkdir
from os.path import getsize

mkdir( LIB_PATH )
mkdir( PRE_INCLUDE_DIRECTORY )
mkdir( INCLUDE_PROJ_DIR )
mkdir( INCLUDE_LIB_DIR )
mkdir( INCLUDE_DIRECTORY )
mkdir( SRC_DIRECTORY )

def generate_file( src_file, out_file ):
	print(f"generating `{out_file}` file...")
	in_file_size =  getsize( src_file )
	with open( src_file, 'r' ) as fin:
		data = fin.read( in_file_size )
		for _old, _new in swap_dict.items():
			data = data.replace( _old, _new )

		with open( out_file, 'w' ) as fout:
			fout.write( data )

generate_file( HEADER_FILE_PATH, join_path( INCLUDE_DIRECTORY, f"{LIB_NAME}.h" ) )
generate_file( SOURCE_FILE_PATH, join_path( SRC_DIRECTORY, f"{LIB_NAME}.cpp" ) )
generate_file( CMAKE_FILE_PATH, join_path( LIB_PATH, f"CMakeLists.txt" ) )

print( "done." )
exit(0)