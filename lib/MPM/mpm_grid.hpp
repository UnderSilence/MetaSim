//
// Created by Metal on 2021/5/17.
//

#ifndef METASIM_MPM_GRID_HPP
#define METASIM_MPM_GRID_HPP

#include "grid.hpp"


template<class TGridData, int Dim>
class MPMGrid : public Grid<TGridData, Dim> {
public:
    using Base = Grid<TGridData, Dim>;

    using T = typename Base::T;
    using TV = typename Base::TV;
    using TVI = typename Base::TVI;

    using Base::nodes_;
    using Base::Xi_;
    using Base::Index;
    using Base::Coord;

    template<class TKernel, class OP>
    void IterateNeighbor(const TV &xp, OP operate) {
        auto[base_node, wp] = TKernel::calc_o_w(xp * inv_dx);

        for (int i = 0; i < TKernel::order + 1; i++)
            for (int j = 0; j < TKernel::order + 1; j++)
                for (int k = 0; k < TKernel::order + 1; k++) {
                    int index = Index(base_node + TVI(i, j, k));
                    T wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);
                    operate(nodes_[index], Xi_[index], wijk);

                }
    }

    template<class TKernel, class OP>
    void IterateNeighborWithGrad(const TV &xp, OP operate) {
        auto[base_node, wp, dwp] = TKernel::calc_o_w_dw(xp * inv_dx);

        for (int i = 0; i < TKernel::order + 1; i++)
            for (int j = 0; j < TKernel::order + 1; j++)
                for (int k = 0; k < TKernel::order + 1; k++) {
                    int index = Index(base_node + TVI(i, j, k));
                    T wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);
                    TV grad_w = {inv_dx * dwp(i, 0) * wp(j, 1) * wp(k, 2),
                                 inv_dx * wp(i, 0) * dwp(j, 1) * wp(k, 2),
                                 inv_dx * wp(i, 0) * wp(j, 1) * dwp(k, 2)};
                    operate(nodes_[index], Xi_[index], wijk, grad_w);
                }
    }

public:
    T dx, inv_dx;
};

#endif //METASIM_MPM_GRID_HPP
