//
// Created by Metal on 2021/5/14.
//

#include "particles.hpp"
#include <iostream>
#include <tbb/parallel_for.h>
#include <tuple>

// #include "MPM/mpm_grid.hpp"
// #include "MPM/mpm_simulator.hpp"
void run_data_container_test() {
  using namespace MS;
  MS::DataContainer container;
  auto rho_tag = TypeTag<float>("rho");
  auto mass_tag = TypeTag<float>("mass");
  auto pf_tag = TypeTag<float>("pf");

  container.append(rho_tag, {0, 5}, 1.0f);
  container.append(mass_tag, {1, 6}, 2.0f);
  container.append(pf_tag, {2, 4}, 3.0f);

  auto subset = container.Subset(rho_tag, mass_tag, pf_tag);
  // auto iter = subset.begin();
  // std::cout << typeid(*iter).name() << std::endl;

  // subset.foreach_element([&](auto& rho, auto& mass, auto& pf) {
  //   std::cout << rho << "," << mass << "," << pf << std::endl;
  // });

  // for (auto it = subset.begin(); it != subset.end(); ++it) {
  //   auto& rho = std::get<0>(*it);
  //   auto& mass = std::get<1>(*it);
  //   auto& pf = std::get<2>(*it);
  //   std::cout << rho << "," << mass << "," << pf << std::endl;
  // }

  // auto test_body = [](auto&& subset) {
  //   std::cout << subset.sub_ranges << ", length: " << subset.size() << std::endl;
  //   subset.foreach_element([&](auto& rho, auto& mass, auto& pf) {
  //     std::cout << rho << "," << mass << "," << pf << std::endl;
  //   });
  // };

  // tbb::parallel_for(subset, test_body);

  for (auto&& [a, b, c] : subset) { std::cout << a++ << "," << b++ << "," << c++ << std::endl; }
  for (auto&& [a, b, c] : subset) { std::cout << a << "," << b << "," << c << std::endl; }
  //   auto& [a, b, c] = *iter;
  //  std::cout << a++ << "," << b++ << "," << c++ << std::endl;
  //  auto iter_copy = iter;
  //  puts(iter_copy == iter? "Yes" : "No");
  //
  //  auto& [d, e, f] = *(++iter);
  //  std::cout << d << "," << e << "," << f << std::endl;
  //  puts(iter_copy == iter? "Yes" : "No");
  //
  //  auto& [i,j,k] = *iter_copy;
  //  std::cout << i << "," << j << "," << k << std::endl;
  //
  //  auto subset2 = container.Subset(rho_tag, mass_tag);
  //  auto ranges = subset.sub_ranges;
  //  std::cout << "ranges:" <<ranges << " length:" << ranges.length() << std::endl;
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
  // run_ranges_test();
  run_data_container_test();
  return 0;
}