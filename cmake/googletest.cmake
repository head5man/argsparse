cmake_minimum_required(VERSION 3.5)

# Use a separate subdirectory for building Google Test so that C++ is not
# globally enabled

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_QUIET FALSE)
set(GOOGLETEST_V1_3_0_HASH "b796f7d44681514f58a683a3a71ff17c94edb0c1")
set(GOOGLETEST_V1_4_0_HASH "f8d7d77c06936315286eb55f8de22cd23c188571")
set(GOOGLETEST_V1_5_2_HASH "b514bdc898e2951020cbdca1304b75f5950d1f59")
set(GOOGLETEST_VERSION ${GOOGLETEST_V1_5_2_HASH})

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG ${GOOGLETEST_VERSION}
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE INTERNAL "Always use msvcrt.dll")

FetchContent_MakeAvailable(googletest)