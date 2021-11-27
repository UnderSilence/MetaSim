
project(LibMetaSim)

include_directories(.)
include_directories(../external/eigen)

file(GLOB_RECURSE HEADERS "*.hpp" "*.h")
file(GLOB_RECURSE SOURCE "*.cpp")

find_package(TBB CONFIG REQUIRED)
add_library(MetaSim STATIC ${SOURCE})
target_link_libraries(MetaSim PUBLIC TBB::tbb)

add_subdirectory(test)

# add_library(${PROJECT_NAME} ${SOURCE} ${HEADERS})