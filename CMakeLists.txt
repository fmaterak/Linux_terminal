cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(terminal)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_STANDARD 11)

find_package(raylib 3.0 QUIET)

# You could change the QUIET above to REQUIRED and remove this if() clause
# This part downloads raylib and builds it if it's not installed on your system
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)

  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/master.tar.gz
  )

  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples

    # build raylib
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})

  endif()

endif()

# This is the main part:
include_directories("inc")
file(GLOB Sources terminal-main.cpp src/*.cpp src/*/*.cpp src/*/*/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${Sources})
# target_compile_definitions(${PROJECT_NAME} PRIVATE SNAKE_BLOCKS_TEST_MAIN)
target_link_libraries(${PROJECT_NAME} raylib)
