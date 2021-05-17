//
// Created by Metal on 2021/5/15.
//

#ifndef METASIM_META_HPP
#define METASIM_META_HPP

#include <array>
#include <vector>
#include <memory>
#include <unordered_map>

#include <Eigen/Core>

using Real = double;
constexpr int Dim = 3;

template<int Dim, typename Scalar = Real>
using Vec = Eigen::Matrix<Scalar, Dim, 1>;

template<int Row, int Col, typename Scalar = Real>
using Mat = Eigen::Matrix<Scalar, Row, Col>;

#define NO_TBB

#define SIM_LOOP(low, high, op) {     \
    for(int i=(low);i<(high);i++) {   \
        (op)(i);                      \
    }                                 \
}


#endif //METASIM_META_HPP
