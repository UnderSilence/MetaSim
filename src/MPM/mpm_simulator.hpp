//
// Created by metal on 5/17/21.
//

#ifndef METASIM_MPM_SIMULATOR_HPP
#define METASIM_MPM_SIMULATOR_HPP

#include "meta.hpp"
#include "simulator.hpp"
#include "mpm_grid.hpp"

template<int Dim>
class MPMSimulator : public Simulator<Dim> {
public:
    using T = real;
    using TV = Vec<Dim, T>;
    using TVI = Vec<Dim, int>;

    using Base = Simulator<Dim>;
    using Base::cfl;
    using Base::dt;
    using Base::max_dt;
    using Base::particles;

    using Base::AddParticleGroup;
    using Base::GetParticleGroup;

    virtual void Step() override {};

public:

    virtual void Particle2Grid() {};
    virtual void Grid2Particle() {};

public:

};

#endif //METASIM_MPM_SIMULATOR_HPP
