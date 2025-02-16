
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/getopt.cmake)

# Main source directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/.. $ENV{EXTRA_INCLUDES})

list(APPEND SourceFiles
    ${CMAKE_CURRENT_LIST_DIR}/../argsparse.c
    ${CMAKE_CURRENT_LIST_DIR}/../internal_funcs.c
)

add_library(${PROJECT_NAME}-lib ${SourceFiles})

target_include_directories(${PROJECT_NAME}-lib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)