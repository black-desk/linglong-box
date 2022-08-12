cmake_minimum_required(VERSION 3.0)

project(linglong-box)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(VERSION "1.0")

# configure debug mode and asan
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Debug"
      CACHE STRING "Choose Release or Debug" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFMT_HEADER_ONLY")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g -ggdb3")

if(CMAKE_DEBUG_ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address")
endif()

find_package(PkgConfig)
include(GNUInstallDirs)

include_directories(${linglong-box_SOURCE_DIR}/3party)

add_subdirectory(src)
# add_subdirectory(test)
