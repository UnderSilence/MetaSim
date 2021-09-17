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

template <typename Type> class DataArrayIterator;
// entry_index -> data_index -> data
template <typename Type> class DataArray : public DataArrayBase {
public:
  using DataArrayBase::name;
  using DataArrayBase::ranges;

  using iterator = class DataArrayIterator<Type>;
  using value_type = Type;
  using reference = Type &;

  std::vector<Type> array;

  DataArray(const std::string &name, const Ranges &ranges,
            std::vector<Type> &&array)
      : DataArrayBase(name, ranges), array(std::move(array)) {}

  auto begin() -> iterator const { return iterator(this); }

  auto end() -> iterator const { return iterator(this, -1); }
};

// RandomAccessIterator Subset iterator
template <typename Type> class DataArrayIterator {
public:
  using value_type = Type;
  using reference = Type &;
  using RangesIter = Ranges::iterator;
  using DataIter = typename std::vector<Type>::iterator;

  Ranges &ranges;
  std::vector<Type> &array;

  RangesIter ranges_iter;
  DataIter data_iter;
  int entry_id;

  DataArrayIterator(const DataArray<Type> &self, int entry_id = 0)
      : ranges(self.ranges), array(self.array), ranges_iter(ranges.begin()),
        data_iter(self.begin()), entry_id(entry_id) {}

  // move entry to dst, redirect data_iter & ranges_iter simultaneously

  auto operator*() -> reference { return *data_iter; }

  int step_entry(int step) {
    while (ranges_iter != ranges.end() &&
           entry_id + step > ranges_iter->upper) {
      auto diff = ranges_iter->upper - entry_id;
      step -= diff;
      data_iter += diff;
      entry_id = (++ranges_iter)->lower;
    }

    if (ranges_iter == ranges.end() && step > 0) {
      entry_id = -1;
      data_iter = array.end();
    } else {
      entry_id += step;
      data_iter += step;
    }
    return entry_id;
  }
};

} // namespace MS

#endif