#ifndef METASIM_DATA_ARRAY_HPP
#define METASIM_DATA_ARRAY_HPP

#include "Core/ranges.hpp"
#include <string>
#include <vector>

namespace MS {
class DataArrayBase {
public:
  std::string name;
  MS::Ranges ranges;

  DataArrayBase(const std::string &name, const Ranges &ranges)
      : name(name), ranges(ranges) {}
};

template <typename Type, typename A = std::allocator<Type>>
class DataArray : public DataArrayBase {
public:
  using DataArrayBase::name;
  using DataArrayBase::ranges;

  // using iterator = DataArrayIterator<Type>;
  // using const_iterator = DataArrayIterator<Type>;

  using value_type = typename A::value_type;
  using reference = typename A::reference;
  using const_reference = typename A::const_reference;
  using difference_type = typename A::difference_type;
  using size_type = typename A::size_type;

  std::vector<Type, A> array;

  DataArray(const std::string &name, const Ranges &ranges,
            std::vector<Type> &&array)
      : DataArrayBase(name, ranges), array(std::move(array)) {}

  auto cbegin() const {
    return const_iterator(ranges.cbegin(), array.cbegin());
  }
  auto cend() const { return const_iterator(ranges.cend(), array.cend()); }

  auto begin() { return iterator(ranges.begin(), array.begin()); }
  auto begin() const { return const_iterator(ranges.cbegin(), array.cbegin()); }

  auto end() { return iterator(ranges.end(), array.end()); }
  auto end() const { return const_iterator(ranges.cend(), array.cend()); }

  class iterator {
    using range_pointer = std::weak_ptr<Ranges>;
    using data_pointer = std::weak_ptr<Type>;
    // using iterator_category = std::forward_iterator_tag;
    // pointer ptr_;

  public:
    iterator() = default;
    iterator(const iterator &) = default;
    ~iterator() = default;

    using RangesIter = Ranges::iterator;
    using ConstRangesIter = Ranges::const_iterator;
    using DataIter = typename std::vector<Type>::iterator;
    using ConstDataIter = typename std::vector<Type>::const_iterator;

    Ranges::const_iterator ranges_iter;

    range_pointer ptr_range;
    data_pointer ptr_data;

    DataIter data_iter;
    int entry_id;

    iterator(const ConstRangesIter &ranges_iter, const DataIter &data_iter)
        : ranges_iter(ranges_iter), ptr_range(ranges_iter), ,data_iter(data_iter),
          entry_id(ranges_iter->lower) {
    }

    // move entry to dst, redirect data_iter & ranges_iter simultaneously

    auto operator*() -> reference { return *data_iter; }
    auto operator++() -> reference {
      step_entry(1);
      return *this;
    }
    auto operator+=(difference_type step) {
      step_entry(step);
      return *this;
    }

    bool operator==(const iterator &rhs) const {
      return entry_id == rhs.entry_id;
    }

    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

    int step_entry(difference_type step) {
      while (entry_id + step >= ranges_iter->upper) {
        auto diff = ranges_iter->upper - entry_id;
        step -= diff;
        data_iter += diff;
        entry_id = (++ranges_iter)->lower;
      }
      entry_id += step;
      data_iter += step;
      return entry_id;
    }

    void move_iterator(int dst_entry_id) {
      int step = 0;
      while (dst_entry_id >= ranges_iter->upper) {
        step += ranges_iter->upper - entry_id;
        entry_id = (++ranges_iter)->lower;
      }
      step += dst_entry_id - entry_id;
      entry_id = dst_entry_id;
      data_iter += step;
    }
  };

  using const_iterator = iterator;
};

//template <typename _Ty>
//using DataArrayIterator = typename DataArray<_Ty>::iterator;
//
//template <typename _Ty>
//using ConstDataArrayIterator = typename DataArray<_Ty>::const_iterator;

} // namespace MS

#endif