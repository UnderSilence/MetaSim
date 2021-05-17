//
// Created by Metal on 2021/5/17.
//

#ifndef METASIM_MPM_GRID_HPP
#define METASIM_MPM_GRID_HPP

#include "grid.hpp"

template<class TGridData, int Dim>
class MPMGrid : public Grid<TGridData, Dim> {
public:

    template<class TKernel, bool use_lock, class OP>
    void IterateNeighborWithGrad(const TV& xp, OP operate) {
        auto [base_node, wp] = TKernel::calc_o_w(xp * inv_dx);

        for (int i = 0; i < TKernel::order + 1; i++)
            for (int j = 0; j < TKernel::order + 1; j++)
                for (int k = 0; k < TKernel::order + 1; k++) {
                    int index = calc_index(base_node + TVI(i, j, k));
                    T wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);

                    if constexpr (use_lock) {
                        operate(node_[index], Xi_[index], wijk,
                                grid_mutexs[index]);
                    } else {
                        operate(grid_attrs[index], grid_Xi[index], wijk);
                    }
                }
    }

    template<class TKernel, bool use_lock, class OP>
    void IterateNeighbor(const TV& xp, OP operate) {

    }
protected:
    T dx, inv_dx;

};

#endif //METASIM_MPM_GRID_HPP
