//
// Created by Metal on 2021/5/14.
//

#ifndef METASIM_SIMULATOR_HPP
#define METASIM_SIMULATOR_HPP

#include "core/grid.hpp"
#include "core/meta.hpp"
#include "core/particles.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"

// T: resolution, Dim: dimension
template<typename T, int Dim>
class Simulator {
public:
  // return group_id

  using frame_callback_t = std::function<void(int)>;
  std::vector<frame_callback_t> frame_begin_callbacks;
  std::vector<frame_callback_t> frame_end_callbacks;
  // frame, total_time
  using timestep_callback_t = std::function<void(int, int)>;
  std::vector<timestep_callback_t> step_begin_callbacks;
  std::vector<timestep_callback_t> step_end_callbacks;

  // pure
  virtual void AdvanceFrame() = 0;
  virtual void AdvanceStep() = 0;
  virtual void Initialize() = 0;

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

#endif   // METASIM_SIMULATOR_HPP
