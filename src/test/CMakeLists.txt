project(MetaSimTest)

add_executable(tbb_test tbb_parallel.cpp)
target_link_libraries(tbb_test PRIVATE MetaSim)

add_executable(container_test data_test.cpp)
target_link_libraries(container_test PRIVATE MetaSim)