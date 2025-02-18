cmake_minimum_required(VERSION 3.5)

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_executable(${PROJECT_NAME}-example example.c)
target_link_libraries(${PROJECT_NAME}-example ${PROJECT_NAME}-lib)