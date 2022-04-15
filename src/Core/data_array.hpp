#ifndef METASIM_DATA_ARRAY_HPP
#define METASIM_DATA_ARRAY_HPP

#include "Core/range_set.hpp"
#include "Utils/logger.hpp"
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

template<typename Type>
class DataArrayIterator;

template<typename Type, typename A = std::allocator<Type>>
class DataArray : public DataArrayBase {
public:
  // using iterator = DataArrayIterator<Type>;
  // using const_iterator = DataArrayIterator<Type>;

  using value_type = Type;
  using reference = Type&;
  using pointer = Type*;
  using const_reference = const Type&;
  using size_type = size_t;

  using iterator = DataArrayIterator<Type>;
  using const_iterator = DataArrayIterator<Type>;

  using DataArrayBase::name;
  using DataArrayBase::ranges;
  std::vector<Type, A> data;

  DataArray(const std::string& name, const RangeSet& ranges, std::vector<Type>&& array)
    : DataArrayBase(name, ranges)
    , data(std::move(array)) {}

  // auto cbegin() const { return const_iterator(data0, 0); }
  // auto cend() const { return const_iterator(*this, -1); }
  auto begin() { return iterator(data.begin(), ranges.begin(), 0); }
  auto end() {
    // auto prev_ranges_end = std::prev(ranges.end());
    return iterator(data.end(), ranges.end(), 0);
  }
  auto begin() const { return const_iterator(data.begin(), ranges.begin(), 0); }
  auto end() const {
    // auto prev_ranges_end = std::prev(ranges.end());
    return iterator(data.end(), ranges.end(), 0);
  }
  auto size() const { return data.size(); }

  // update values in range by data
  auto update(const Range& range, std::vector<Type>&& array) {
    // erase_range(range);
    // insert_data(merge())
    // intersections need to remove, and it is contiguous
    auto inter_ranges = ranges & range;
    if (inter_ranges.length()) {
      // if intersections exists, then remove them
      auto x_begin = inter_ranges.begin()->lower;
      auto p_del = ranges.query_offset(x_begin);
      auto len_del = inter_ranges.length();
      data.erase(data.begin() + p_del, data.begin() + len_del);
    }

    int p_insert = ranges.query_offset(range.lower);
    ranges.merge(range);
  }

  auto append(const Range& range, std::vector<Type>&& array) {
    ranges.merge(range);
    data.insert(
      data.end(), std::make_move_iterator(array.begin()), std::make_move_iterator(array.end()));
  }
};

template<class T>
class DataArrayIterator {
public:
  // iterator_traits definitions
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T;
  using reference = T&;
  using pointer = T*;
  using difference_type = ptrdiff_t;

  typename std::vector<T>::iterator data_iter;
  RangeSet::iterator range_iter;
  difference_type entry_offset;

  DataArrayIterator() = default;
  DataArrayIterator(const DataArrayIterator&) = default;
  DataArrayIterator(typename std::vector<T>::iterator data_iter, RangeSet::iterator range_iter,
                    difference_type entry_offset)
    : data_iter(data_iter)
    , range_iter(range_iter)
    , entry_offset(entry_offset) {}

  bool operator==(const DataArrayIterator<T>& other) {
    return other.range_iter ==
               range_iter /*for compare begins, ends cause they have no legal entry()*/
             ? (other.entry_offset == entry_offset ? true : other.entry() == entry())
             : false;
  }
  bool operator!=(const DataArrayIterator<T>& other) {
    return !operator==(*this, other);   // need other!=*this?
  }

  bool operator<(const DataArrayIterator<T>& other) { return data_iter < other.data_iter; }

  // dereferenced
  reference operator*() { return *data_iter; }
  auto operator++() { return *this += 1; }
  auto operator+=(difference_type offset) {
    // offset > 0
    while (offset > 0 && (entry_offset == -1 || entry() + offset >= range_iter->upper)) {
      // promise step_size > 0 && range_iter exists
      offset -= _to_next_first();
    }
    _move_in_range(offset);
    return *this;
  }
  auto operator--() { return *this -= 1; }
  auto operator-=(difference_type offset) {
    // offset > 0
    while (offset > 0 && (entry_offset == 0 || entry() - offset < range_iter->lower)) {
      offset += _to_prev_last();
    }
    _move_in_range(-offset);
    return *this;
  }

  int entry() const {
    return (entry_offset < 0 ? range_iter->upper : range_iter->lower) + entry_offset;
  }

  auto advance(difference_type step_size) {
    return step_size > 0 ? *this += step_size : *this -= step_size;
  }

  // avoid using this iff you know what r u doing.
  // make sure that abs(entry() - target_entry) != 0, and target_entry is legal
  // basically it's somewhat a hack
  template<bool is_forward = true>
  auto move_entry_to(int target_entry) {
    if constexpr (is_forward) {
      // META_TRACE("move target to {} forward", target_entry);
      while (entry_offset == -1 || target_entry >= range_iter->upper) {
        _to_next_first();   // O(1)
      }
    } else {
      // META_TRACE("move target to {} backward", target_entry);
      while (entry_offset == 0 || target_entry < range_iter->lower) {
        _to_prev_last();
      }
    }
    _to_entry_in_range(target_entry);
  }

private:
  // move entry() to prev(range_iter)->upper - 1
  // return real index offset in data array
  inline int _to_prev_last() {
    auto offset = -(entry_offset < 0 ? range_iter->length() + entry_offset : entry_offset) - 1;
    // META_INFO("data_iter offset {}", offset);
    data_iter += offset;
    --range_iter;
    entry_offset = -1;
    return offset;
  }

  // move entry() to next(range_iter)->lower
  // return real index offset in data array
  inline int _to_next_first() {
    auto offset = (entry_offset < 0 ? -entry_offset : range_iter->length() - entry_offset);
    // META_INFO("data_iter offset {}", offset);
    data_iter += offset;
    ++range_iter;
    entry_offset = 0;
    return offset;
  }

  inline void _to_entry_in_range(int _target_entry) { _move_in_range(_target_entry - entry()); }

  inline void _move_in_range(int _offset) {
    data_iter += _offset;
    entry_offset += _offset;
  }

  // make entry_offset positive
  void _make_entry_offset_position() {
    if (entry_offset < 0) {
      entry_offset = range_iter->upper + entry_offset;
    }
  }
};

}   // namespace MS

#endif