cmake_minimum_required(VERSION 3.5)
project(argsparse)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/argsparse-lib.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/example)

enable_testing()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
