cmake_minimum_required(VERSION 3.16)
project(onvif-discover)

set(CMAKE_CXX_STANDARD 11)

add_definitions("-Wall -g")
aux_source_directory(. SRC_LIST)
add_executable(onvif-discover  ${SRC_LIST} )
