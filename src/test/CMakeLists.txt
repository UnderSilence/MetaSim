project(MetaSimTest)

find_package(TBB CONFIG REQUIRED)

add_executable(tbb_test tbb_parallel.cpp)
target_link_libraries(tbb_test PRIVATE TBB::tbb)