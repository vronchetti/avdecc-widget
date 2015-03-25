cmake_minimum_required (VERSION 2.8) 
project (avdecc_gui)
add_subdirectory("avdecc-widget")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../avdecc-lib" avdecc-lib)
