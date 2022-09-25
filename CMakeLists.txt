cmake_minimum_required(VERSION 3.0)

project(
  linglong-box
  VERSION 0.1.0
  DESCRIPTION "OCI runtime for linglong"
  HOMEPAGE_URL "https://github.com/linuxdeepin/linglong-box"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(./third_party)

add_subdirectory(src)
# add_subdirectory(test)
