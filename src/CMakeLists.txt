
project(LibMetaSim)

include_directories(.)
include_directories(../external/eigen)
include_directories(../external/spdlog/include)

file(GLOB_RECURSE HEADERS "*.hpp" "*.h")
file(GLOB_RECURSE SOURCE "*.cpp")

message("cmake project directory ${CMAKE_SOURCE_DIR}")
configure_file(
    "${PROJECT_SOURCE_DIR}/Core/forward.hpp.in"
    "${PROJECT_SOURCE_DIR}/Core/forward.hpp")


find_package(TBB CONFIG REQUIRED)
add_library(MetaSim STATIC ${SOURCE})
target_link_libraries(MetaSim PUBLIC TBB::tbb spdlog)

target_precompile_headers(MetaSim PUBLIC "${PROJECT_SOURCE_DIR}/meta_pch.hpp")
add_subdirectory(test)

# add_library(${PROJECT_NAME} ${SOURCE} ${HEADERS})