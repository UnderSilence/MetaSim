//
// Created by Metal on 2021/5/14.
//

#include <iostream>
#include "particles.hpp"

auto main() -> int {
    std::cout << "Hello MetaSim!" << std::endl;

    ParticleContainer particle_container;
    auto& mass_array = particle_container.SetAttrArray<Real>("mass");
    int demo_size = 10;
    mass_array.resize(demo_size);
    for(int i=0;i<demo_size;i++) {
        mass_array[i] = i + 1;
    }

    auto& mass_array_2 = particle_container.GetAttrArray<Real>("mass");
    printf("mass_array_size = %d\n", (int)mass_array_2.size());
    for(int i=0;i<mass_array_2.size();i++) {
        printf("mass[%d] = %f\n", i, mass_array_2[i]);
    }


    return 0;
}