//
// Created by Metal on 2021/5/14.
//

#ifndef METASIM_SIMULATOR_HPP
#define METASIM_SIMULATOR_HPP

#include "meta.hpp"
#include "particles.hpp"
#include "grid.hpp"

template<int Dim>
class Simulator {
public:
    float dt;
    float cfl;
    float max_dt;

    // return group_id
    size_t AddParticleGroup(const ParticleContainer& part_ctn) {
        particles.push_back(part_ctn);
        return particles.size();
    }

    ParticleContainer& GetParticleGroup(int group_id) {
        return particles[group_id];
    }

    virtual void Step() = 0;

public:
    std::vector<ParticleContainer> particles;
};

#endif //METASIM_SIMULATOR_HPP
