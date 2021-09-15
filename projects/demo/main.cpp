//
// Created by Metal on 2021/5/14.
//

#include "particles.hpp"
#include <iostream>
// #include "MPM/mpm_grid.hpp"
// #include "MPM/mpm_simulator.hpp"

void run_ranges_test() {

  MS::Ranges ranges({2, 6});
  MS::Ranges ranges_1({7, 8});
  std::cout << ranges << std::endl;
  ranges.erase({3, 4});
  std::cout << ranges << std::endl;
  ranges.erase({5, 6});
  std::cout << ranges << std::endl;

  ranges.intersect(ranges_1);
  std::cout << ranges << std::endl;
  ranges.merge({2, 6});
  std::cout << ranges << std::endl;
}

void run_data_container_test() {
  using namespace MS;
  MS::DataContainer container;
  auto attr_tag = AttributeTag<float>("rho");
  container.append(attr_tag, {0, 5}, 1.0f);
  // container.append(MS::AttributeTag<>)
  auto &attr_arr = container.get_array(attr_tag);
  return;
}

auto main() -> int {
  std::cout << "Hello MetaSim!" << std::endl;
  /*
      ParticleContainer particle_container;
      auto& mass_array = particle_container.SetAttrArray<real>("mass");
      int demo_size = 10;
      mass_array.resize(demo_size);
      for(int i=0;i<demo_size;i++) {
          mass_array[i] = i + 1;
      }

      auto& mass_array_2 = particle_container.GetAttrArray<real>("mass");
      printf("mass_array_size = %d\n", (int)mass_array_2.size());
      for(int i=0;i<mass_array_2.size();i++) {
          printf("mass[%d] = %f\n", i, mass_array_2[i]);
      }

      struct TestGridData {
          float x,y,z;
          Vec<3, float> test_vec;
      };

      MPMGrid<TestGridData, 3> grid;
      grid.InitializeGrid({5,5,5}, TestGridData());


      MPMSimulator<3> simulator;
  */
  run_data_container_test();
  return 0;
}