#!/usr/bin/python3

import os
from sys import argv
from os.path import join as join_path

PROJECT_NAME = "antybiurokrata"
WINDOWS_DIR_NAME = "windows"
DESTINATION_DIR = f"../../{WINDOWS_DIR_NAME}"

print("checking aruments...")
assert len(argv) == 2, 'not enough arguments, pass window name. Ex: ./create_new_window.py super_dooooooper_window_2'
WINDOW_NAME : str = argv[1]
WINDOW_PATH = join_path(DESTINATION_DIR, WINDOW_NAME)

INPUT_DIRECTORY = join_path( os.curdir, 'example_window.in' )

print("checking cwd...")
assert os.path.exists(INPUT_DIRECTORY), f'no "{INPUT_DIRECTORY}" found'

print("creating paths...")
SOURCE_FILE_PATH = join_path( INPUT_DIRECTORY, 'win.cpp.in' )
HEADER_FILE_PATH = join_path( INPUT_DIRECTORY, 'win.h.in' )
UI_FILE_PATH = join_path( INPUT_DIRECTORY, 'win.ui.in' )
CMAKE_FILE_PATH = join_path( INPUT_DIRECTORY, 'win.cmake.in' )

PRE_INCLUDE_DIRECTORY = join_path( WINDOW_PATH, "include" )
INCLUDE_PROJ_DIR = join_path( PRE_INCLUDE_DIRECTORY, PROJECT_NAME )
INCLUDE_WIN_DIR = join_path( INCLUDE_PROJ_DIR, WINDOWS_DIR_NAME )
INCLUDE_DIRECTORY = join_path( INCLUDE_WIN_DIR, WINDOW_NAME )
SRC_DIRECTORY = join_path( WINDOW_PATH, "src" )

swap_dict = {
	"ExWindow": WINDOW_NAME
}

print("creating directory structure...")
from os import mkdir
from os.path import getsize

mkdir( WINDOW_PATH )
mkdir( PRE_INCLUDE_DIRECTORY )
mkdir( INCLUDE_PROJ_DIR )
mkdir( INCLUDE_WIN_DIR )
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

generate_file( HEADER_FILE_PATH, join_path( INCLUDE_DIRECTORY, f"{WINDOW_NAME}.h" ) )
generate_file( SOURCE_FILE_PATH, join_path( SRC_DIRECTORY, f"{WINDOW_NAME}.cpp" ) )
generate_file( UI_FILE_PATH, join_path( SRC_DIRECTORY, f"{WINDOW_NAME}.ui" ) )
generate_file( CMAKE_FILE_PATH, join_path( WINDOW_PATH, f"CMakeLists.txt" ) )

print( "done." )
exit(0)