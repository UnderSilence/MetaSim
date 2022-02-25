//
// Created by Metal on 2021/5/15.
//

#ifndef METASIM_META_HPP
#define METASIM_META_HPP

#include <Eigen/Core>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


using real = double;

template<int Dim, typename Scalar = real>
using Vec = Eigen::Matrix<Scalar, Dim, 1>;

template<int Row, int Col, typename Scalar = real>
using Mat = Eigen::Matrix<Scalar, Row, Col>;

// context controller (program exec cwd)
// global singleton
struct Context {
  std::string abs_project_dir{"D:/Workspace/.Simulations/MetaSim"};
  std::string output_dir, data_dir;
  Context() {
    output_dir = abs_project_dir + "/output";
    data_dir = abs_project_dir + "/data";
  }
} context;

#endif   // METASIM_META_HPP
