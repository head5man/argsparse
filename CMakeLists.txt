cmake_minimum_required(VERSION 3.3.2)
project(argsparse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(NOT googletest_POPULATED)
    include(${CMAKE_CURRENT_LIST_DIR}/googletest.cmake)
endif()

# Main source directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

list(APPEND SourceFiles
    ${CMAKE_CURRENT_LIST_DIR}/argsparse.c
)

list(APPEND TestFiles
    ${CMAKE_CURRENT_LIST_DIR}/test/argsparseTests.cpp
)

# skeleton sources as library
add_library(${PROJECT_NAME}-lib ${SourceFiles})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.. ${log4cplus_SOURCE_DIR}/include)

# skeleton test executable
add_executable(${PROJECT_NAME}-test ${CMAKE_CURRENT_LIST_DIR}/test/TestMain.cpp ${TestFiles})
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME}-lib gtest gmock_main)
target_include_directories(${PROJECT_NAME}-test PUBLIC ${log4cplus_SOURCE_DIR}/include ${googletest_SOURCE_DIR}/googlemock/include)

add_test(NAME ${PROJECT_NAME}-test COMMAND ${PROJECT_NAME}-test)

#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
