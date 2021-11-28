//
// Created by Metal on 2021/5/15.
//

#ifndef METASIM_META_HPP
#define METASIM_META_HPP

#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <Eigen/Core>

using real = double;

template<int Dim, typename Scalar = real>
using Vec = Eigen::Matrix<Scalar, Dim, 1>;

template<int Row, int Col, typename Scalar = real>
using Mat = Eigen::Matrix<Scalar, Row, Col>;

#define NO_TBB

#define SIM_LOOP(low, high, op) {     \
    for(int i=(low);i<(high);i++) {   \
        (op)(i);                      \
    }                                 \
}


// context controller (program exec cwd)
// global singleton
class Context { 
    std::string AbsProjDir = "D:/Workspace/.Simulations/MetaSim";
} context;

#endif //METASIM_META_HPP
