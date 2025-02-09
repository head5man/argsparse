
if(MSVC)
  include(FetchContent)
  
  FetchContent_Declare(
    getopt
    GIT_REPOSITORY https://github.com/skandhurkat/Getopt-for-Visual-Studio.git
  )
  FetchContent_GetProperties(getopt)

  if (NOT getopt_POPULATED)
    FetchContent_Populate(getopt)
  endif()

  message(STATUS "getopt ${getopt_SOURCE_DIR}")
  set(ENV{EXTRA_INCLUDES} "$ENV{EXTRA_INCLUDES} ${getopt_SOURCE_DIR}")
endif()