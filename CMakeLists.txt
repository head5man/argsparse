cmake_minimum_required(VERSION 3.5)
project(argsparse)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/argsparse-lib.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)

# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED YES)

# include(${CMAKE_CURRENT_LIST_DIR}/cmake/googletest.cmake)

# include(${CMAKE_CURRENT_LIST_DIR}/cmake/argsparse-lib.cmake)

# # Main source directory
# include_directories(${CMAKE_CURRENT_LIST_DIR}/.)

# list(APPEND TestFiles
#     ${CMAKE_CURRENT_LIST_DIR}/test/argsparseTests.cpp
# )

# # Test executable
# add_executable(${PROJECT_NAME}-test ${CMAKE_CURRENT_LIST_DIR}/test/TestMain.cpp ${TestFiles})
# target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME}-lib gtest gmock_main)
# target_include_directories(${PROJECT_NAME}-test PUBLIC ${googletest_SOURCE_DIR}/googlemock/include)

# include(CTest)

# add_test(NAME ${PROJECT_NAME}-test COMMAND ${PROJECT_NAME}-test)
