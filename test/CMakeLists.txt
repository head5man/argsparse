cmake_minimum_required(VERSION 3.5)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/googletest.cmake)

# Main source directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

list(APPEND SourceFiles
    ${CMAKE_CURRENT_LIST_DIR}/TestMain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/argsparseTests.cpp
)

# Test executable
add_executable(${PROJECT_NAME}-test ${SourceFiles})
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME}-lib gtest gmock_main)
target_include_directories(${PROJECT_NAME}-test PUBLIC ${googletest_SOURCE_DIR}/googlemock/include)

include(CTest)

add_test(NAME ${PROJECT_NAME}-test COMMAND ${PROJECT_NAME}-test)
