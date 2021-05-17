project(MetaSim_Demo)

add_executable(${PROJECT_NAME} main.cpp ../../lib/MPM/mpm_simulator.hpp ../../lib/MPM/mpm_simulator.cpp)

# target_link_libraries(${PROJECT_NAME} PUBLIC LibMetaSim)