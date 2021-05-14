//
// Created by Metal on 2021/5/15.
//

#ifndef METASIM_META_HPP
#define METASIM_META_HPP

#include <memory>
#include <unordered_map>

#include <Eigen/Core>

using Real = double;

template<int Dim>
using Vec = Eigen::Matrix<Real, Dim, 1>;

template<int Dim>
using Mat = Eigen::Matrix<Real, Dim, Dim>;

#endif //METASIM_META_HPP
