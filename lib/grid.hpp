//
// Created by Metal on 2021/5/14.
//

#ifndef METASIM_GRID_HPP
#define METASIM_GRID_HPP

#include "meta.hpp"

class IGridBase {
public:
    virtual ~IGridBase() = default;
};

template<typename TGridData, int Dim>
class Grid : public IGridBase {

public:
    using T = Real;
    using TVI = Vec<Dim, int>;
    using TV = Vec<Dim, T>;

    Grid() = default;

    TGridData& at(const TVI& coord) {
        return nodes_[Index(coord)];
    }

    size_t Index(const TVI &coord) {
        size_t stride = 1;
        size_t result = 0;
        for (int i = Dim - 1; i >= 0; i--) {
            result += stride * coord(i);
            stride *= shape_[i]; // warning: no overflow check
        }
        return result;
    };

    TVI Coord(size_t index) {
        size_t stride = total_size_;
        TVI result;
        for (int i = 0; i < Dim; i++) {
            stride /= shape_[i];
            result(i) = index / stride;
            index %= stride;
        }
        return result;
    }

    virtual void InitializeGrid(const std::array<size_t, Dim> &shape, const TGridData &init_value) {
        total_size_ = 1;
        for (int i = 0; i < Dim; i++) {
            shape_[i] = shape[i];
            total_size_ *= shape_[i];
        }

        nodes_.resize(total_size_);
        Xi_.resize(total_size_);

        for (int i = 0; i < total_size_; i++) {
            Xi_[i] = Coord(i);
            nodes_[i] = init_value;
            printf("Xi of %d is (%d, %d, %d)\n", i, Xi_[i](0), Xi_[i](1), Xi_[i](2));
        }
    };

    template<typename OP>
    void IterateAllGrid(OP operate) {
        SIM_LOOP(0, nodes_.size(), [&](int i) {
            operate(nodes_[i], Xi_[i], i);
        });
    }

    template<typename OP>
    void IterateActiveGrid(OP operate) {
        SIM_LOOP(0, active_idx_.size(), [&](int i) {
            auto idx = active_idx_[i];
            operate(nodes_[idx], Xi_[idx], idx);
        });
    }

    template<typename OP>
    void IterateAllGridWithCheck(OP operate) {
        active_idx_.clear();
        SIM_LOOP(0, nodes_.size(), [&](int i) {
            if (operate(nodes_[i], Xi_[i], i)) {
                active_idx_.push_back(i);
            }
        });
    }

protected:
    size_t total_size_ = 0;
    std::array<size_t, Dim> shape_;
    std::vector<TGridData> nodes_;
    std::vector<TVI> Xi_;
    std::vector<size_t> active_idx_;
};


#endif //METASIM_GRID_HPP
