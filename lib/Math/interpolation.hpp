//
// Created by Metal on 2021/5/17.
//

#ifndef METASIM_INTERPOLATION_HPP
#define METASIM_INTERPOLATION_HPP

#include "meta.hpp"


struct InterpolationKernelBase {
public:
    virtual ~InterpolationKernelBase() = default;
};

template<typename KernelImpl, int Dim, int Order>
struct MPMInterpolationKernel : InterpolationKernelBase {
public:
    using T = Real;
    using TVI = Vec<Dim, int>;
    using TV = Vec<Dim, T>;

    using KernelVec = Vec<Order + 1>;
    using KernelMat = Mat<Order + 1, Dim>;

    constexpr static const int order = Order;

    // pass in particle_pos in Grid Space (aka. xp / dx)
    static std::tuple<TVI, KernelMat, KernelMat, KernelMat> calc_o_w_dw_ddw(
            const TV &xp_div_dx) {
        KernelMat w, dw, ddw;
        auto o = calc_base_node(xp_div_dx);
        for (int i = 0; i < Dim; i++) {
            w.col(i) = calc_weight(o(i), xp_div_dx(i));
            dw.col(i) = calc_weight_grad(o(i), xp_div_dx(i));
            ddw.col(i) = calc_weight_hessian(o(i), xp_div_dx(i));
        }
        return {o, w, dw, ddw};
    }

    static std::tuple<TVI, KernelMat, KernelMat> calc_o_w_dw(
            const TV &xp_div_dx) {
        KernelMat w, dw, ddw;
        auto o = calc_base_node(xp_div_dx);
        for (int i = 0; i < Dim; i++) {
            w.col(i) = calc_weight(o(i), xp_div_dx(i));
            dw.col(i) = calc_weight_grad(o(i), xp_div_dx(i));
        }
        return {o, w, dw};
    }

    static std::tuple<TVI, KernelMat> calc_o_w(const TV &xp_div_dx) {
        KernelMat w, dw, ddw;
        auto o = calc_base_node(xp_div_dx);
        for (int i = 0; i < Dim; i++) {
            w.col(i) = calc_weight(o(i), xp_div_dx(i));
        }
        return {o, w};
    }
    /*
      should not contain STATE for multi-threading usage
      Vec<Dim> center;
      VecI<Dim> base_node;
   */

    // https://www.quora.com/Why-can%E2%80%99t-we-declare-static-function-as-virtual-in-C++
    // https://stackoverflow.com/questions/1820477/c-static-virtual-members
    // https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
    static KernelVec calc_weight(T o, T x) {
        // prevent infinite recursion
        // https://stackoverflow.com/questions/39918941/is-function-pointer-comparison-in-a-constexpr-function-allowed
        // static_assert(&KernelImpl::calc_weight != &calc_weight);
        return KernelImpl::calc_weight(o, x);
    };
    static KernelVec calc_weight_grad(T o, T x) {
        // static_assert(&KernelImpl::calc_weight_grad != &calc_weight_grad);
        return KernelImpl::calc_weight_grad(o, x);
    };
    static KernelVec calc_weight_hessian(T o, T x) {
        // static_assert(&KernelImpl::calc_weight_hessian != &calc_weight_hessian);
        return KernelImpl::calc_weight_hessian(o, x);
    };

    static TVI calc_base_node(const Vec<Dim> &center) {
        return Eigen::floor(center.array() - 0.5 * (Order - 1)).cast<int>();
    }
};

template<int Dim>
struct QuadraticKernel : public MPMInterpolationKernel<QuadraticKernel, Dim, 2> {

    static KernelVec calc_weight(T o, T x) {
        // +-(o)------(o+1)--(x)--(o+2)-+
        T d0 = x - o;
        T d1 = d0 - 1;
        T d2 = 1 - d1;

        return {T(0.5) * T(1.5 - d0) * T(1.5 - d0), T(0.75) - d1 * d1,
                T(0.5) * T(1.5 - d2) * T(1.5 - d2)};
    }

    static KernelVec calc_weight_grad(T o, T x) {
        T d0 = x - o;
        T d1 = d0 - 1;
        T d2 = 1 - d1;

        return {d0 - T(1.5), -2 * d1, T(1.5) - d2};
    }

    static KernelVec calc_weight_hessian(T o, T x) {
        return {1, -2, 1};
    }
};

template<int Dim>
struct CubicKernel : public MPMInterpolationKernel<CubicKernel, Dim, 3> {

    static KernelVec calc_weight(T o, T x) {
        T d0 = x - o;
        T z = 2 - d0;
        T d1 = d0 - 1;
        T d2 = 1 - d1;
        T d3 = 1 + d2;
        T zz = 2 - d3;

        return {T(z * z * z / 6), T((0.5 * d1 - 1) * d1 * d1 + 2.0 / 3),
                T((0.5 * d2 - 1) * d2 * d2 + 2.0 / 3), T(zz * zz * zz / 6)};
    }

    static KernelVec calc_weight_grad(T o, T x) {
        T d0 = x - o;
        T z = 2 - d0;
        T d1 = d0 - 1;
        T d2 = 1 - d1;
        T d3 = 1 + d2;
        T zz = 2 - d3;

        return {T(-0.5 * z * z), T((1.5 * d1 - 2) * d1), T((-1.5 * d2 + 2) * d2),
                T(0.5 * zz * zz)};
    }

    static KernelVec calc_weight_hessian(T o, T x) {
        T d0 = x - o;
        T z = 2 - d0;
        T d1 = d0 - 1;
        T d2 = 1 - d1;
        T d3 = 1 + d2;
        T zz = 2 - d3;

        return {T(z), T(3 * d1 - 2), T(3 * d2 - 2), T(zz)};
    }

};


#endif //METASIM_INTERPOLATION_HPP
