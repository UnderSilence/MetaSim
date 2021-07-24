project(MetaSim_Demo)

add_executable(${PROJECT_NAME} main.cpp ../../src/MPM/mpm_simulator.hpp ../../src/MPM/mpm_simulator.cpp)

# target_link_libraries(${PROJECT_NAME} PUBLIC LibMetaSim)