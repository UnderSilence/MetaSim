//
// Created by metal on 5/17/21.
//

#ifndef METASIM_MPM_SIMULATOR_HPP
#define METASIM_MPM_SIMULATOR_HPP

#include "core/meta.hpp"
#include "core/simulator.hpp"
#include "mpm_grid.hpp"

namespace MS {

template<typename T, int Dim>
class MPMSimulator : public Simulator<T, Dim> {
public:
  using TV = Vec<Dim, T>;
  using TVI = Vec<Dim, int>;

  using Base = Simulator<T, Dim>;
  using Base::dt;
  using Base::max_dt;

  virtual void advance_frame() override;
  virtual void advance_step() override;

public:
  T cfl;

public:
};
}   // namespace MS

#endif   // METASIM_MPM_SIMULATOR_HPP
