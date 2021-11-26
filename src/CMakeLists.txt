
project(LibMetaSim)

include_directories(.)
include_directories(../external/eigen)

file(GLOB_RECURSE HEADERS "*.hpp" "*.h")
file(GLOB_RECURSE SOURCE "*.cpp")

add_subdirectory(test)

# add_library(${PROJECT_NAME} ${SOURCE} ${HEADERS})