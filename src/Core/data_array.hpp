#ifndef METASIM_DATA_ARRAY_HPP
#define METASIM_DATA_ARRAY_HPP

#include "Core/range_set.hpp"
#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

namespace MS {
class DataArrayBase {
public:
  std::string name;
  MS::RangeSet ranges;

  DataArrayBase(const std::string& name, const RangeSet& ranges)
    : name(name)
    , ranges(ranges) {}
};

template<typename Type, typename A = std::allocator<Type>>
class DataArray : public DataArrayBase {
public:
  // using iterator = DataArrayIterator<Type>;
  // using const_iterator = DataArrayIterator<Type>;

  using value_type = typename A::value_type;
  using reference = typename A::reference;
  using const_reference = typename A::const_reference;
  using difference_type = typename A::difference_type;
  using size_type = typename A::size_type;

  using DataArrayBase::name;
  using DataArrayBase::ranges;
  std::vector<Type, A> array;

  DataArray(const std::string& name, const RangeSet& ranges, std::vector<Type>&& array)
    : DataArrayBase(name, ranges)
    , array(std::move(array)) {}

  auto cbegin() const { return const_iterator(*this, 0); }
  auto cend() const { return const_iterator(*this, size()); }

  auto begin() { return iterator(*this, 0); }
  auto end() { return iterator(*this, size()); }

  auto begin() const { return const_iterator(*this, 0); }
  auto end() const { return const_iterator(*this, size()); }

  auto size() const { return array.size(); }

  class iterator {
  public:
    iterator() = default;
    iterator(const iterator&) = default;
    ~iterator() = default;

    //    using RangeSetIter = RangeSet::iterator;
    //    using ConstRangeSetIter = RangeSet::const_iterator;
    //    using DataIter = typename std::vector<Type>::iterator;
    //    using ConstDataIter = typename std::vector<Type>::const_iterator;
    using iterator_category = std::forward_iterator_tag;
    using ranges_iterator = RangeSet::iterator;
    using data_iterator = typename std::vector<Type>::iterator;

    ranges_iterator ranges_iter;
    data_iterator data_iter;
    // Global id to specify element
    int entry_id;

    iterator(DataArray& self, size_type index) {
      // restore entryId ? its ok to compare with end?
      if (index == -1 || index >= self.ranges.length()) {
        // end iterator
        ranges_iter = self.ranges.end();
        data_iter = self.array.end();
        entry_id = self.ranges.back().upper;
      } else {
        ranges_iter = self.ranges.begin();
        data_iter = self.array.begin();
        entry_id = self.ranges.front().lower;
        *this += index;
      }
    }

    // move entry to dst, redirect data_iter & ranges_iter simultaneously
    auto operator*() -> reference { return *data_iter; }
    auto operator++() -> reference { return (*this += 1); }
    auto operator+=(difference_type step) {
      advance(step);
      return *this;
    }

    bool operator==(const iterator& rhs) const { return data_iter == rhs.data_iter; }
    bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

    int advance(int step) {
      while (step > 0 && entry_id + step >= ranges_iter->upper) {
        auto diff = ranges_iter->upper - entry_id;
        step -= diff;
        data_iter += diff;
        entry_id = (++ranges_iter)->lower;
      }
      entry_id += step;
      data_iter += step;
      return entry_id;
    }

    // What if target_entry outside of the boundary?
    void advance_to(int target_entry) {
      auto step = 0;
      while (target_entry >= ranges_iter->upper) {
        step += ranges_iter->upper - entry_id;
        entry_id = (++ranges_iter)->lower;
      }
      step += target_entry - entry_id;
      entry_id = target_entry;
      data_iter += step;
    }
  };

  using const_iterator = iterator;
};

// template <typename _Ty>
// using DataArrayIterator = typename DataArray<_Ty>::iterator;
//
// template <typename _Ty>
// using ConstDataArrayIterator = typename DataArray<_Ty>::const_iterator;

}   // namespace MS

#endif