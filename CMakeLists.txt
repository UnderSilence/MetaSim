CMAKE_MINIMUM_REQUIRED(VERSION 3.16.0)

set(TBB_DIR "${CMAKE_SOURCE_DIR}/cmake")

message("${CMAKE_MODULE_PATH}")
project(MetaSim)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)
add_subdirectory(external)
add_subdirectory(projects)