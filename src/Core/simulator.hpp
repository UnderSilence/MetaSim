//
// Created by Metal on 2021/5/14.
//

#ifndef METASIM_SIMULATOR_HPP
#define METASIM_SIMULATOR_HPP

#include "core/meta.hpp"
#include "utils/logger.hpp"
#include "utils/profiler.hpp"

namespace MS {
// T: resolution(float), Dim: dimension
template<typename T, int Dim>
class Simulator {
public:
  // return group_id
  using TV = Vec<Dim, T>;
  using TM = Mat<Dim, Dim, T>;

  using sim_callback_t = std::function<void()>;
  std::vector<sim_callback_t> sim_begin_callbacks;
  std::vector<sim_callback_t> sim_end_callbacks;

  using frame_callback_t = std::function<void(int)>;
  std::vector<frame_callback_t> frame_begin_callbacks;
  std::vector<frame_callback_t> frame_end_callbacks;
  // frame, total_time
  using timestep_callback_t = std::function<void(int, T)>;
  std::vector<timestep_callback_t> step_begin_callbacks;
  std::vector<timestep_callback_t> step_end_callbacks;

  virtual void advance_frame() = 0;
  virtual void advance_step() = 0;
  virtual void initialize() = 0;

  Simulator() {
    if (set_timer) {}
  }

  virtual ~Simulator() {
    if (set_timer) {}
  }

public:
  // some configurations
  bool write_log{true};
  bool write_frame{true};
  bool set_timer{true};

public:
  T dt;
  T max_dt;
  T fps;           // frame per seconds
  int frame_cnt;   // current frame count
  int step_cnt;    // current step count
  T total_time;
};
}   // namespace MS

#endif   // METASIM_SIMULATOR_HPP
