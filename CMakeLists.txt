cmake_minimum_required(VERSION 2.4)

set(CMAKE_C_COMPILER "gcc")

project(hw1)
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src src)

add_executable(hw1 ${src})
