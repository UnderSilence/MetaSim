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
  auto end() { return iterator(data.end(), ranges.end(), 0); }
  auto begin() const { return const_iterator(data.begin(), ranges.begin(), 0); }
  auto end() const { return const_iterator(data.end(), ranges.end(), 0); }
  auto size() const { return data.size(); }

  // update values in range by data
  auto update(const Range& range, std::vector<Type>&& data) {
    // erase_range(range);
    // insert_data(merge())
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
    return *this != other;   // need other!=*this?
  }

  // dereferenced
  reference operator*() { return *data_iter; }
  auto operator++() { return *this += 1; }
  auto operator+=(difference_type offset) {
    // offset > 0
    while (offset > 0 && entry() + offset >= range_iter->upper) {
      // promise step_size > 0 && range_iter exists
      auto jump_count = (entry_offset < 0 ? -entry_offset : range_iter->length() - entry_offset);
      offset -= jump_count;
      data_iter += jump_count;

      range_iter++;
      entry_offset = 0;
    }
    data_iter += offset;
    entry_offset = offset;
    return *this;
  }
  auto operator--() { return *this -= 1; }
  auto operator-=(difference_type offset) {
    // offset > 0
    while (offset > 0 && entry() - offset < range_iter->lower) {
      auto jump_count = (entry_offset < 0 ? range_iter->length() + entry_offset : entry_offset) + 1;
      offset -= jump_count;
      data_iter -= jump_count;

      range_iter--;
      entry_offset = -1;   // -1 means last position in current range
    }
    data_iter += offset;
    entry_offset = offset;
    return *this;
  }

  int entry() const {
    return entry_offset < 0 ? range_iter->upper : range_iter->lower + entry_offset;
  }

  auto advance(difference_type step_size) {
    return step_size > 0 ? *this += step_size : *this -= step_size;
  }

  // need ensure target_entry is legal
  auto move_entry_to(int target_entry) {
    META_INFO("Try move to target_entry {}, current range {}, entry_offset {}", target_entry, *range_iter, entry_offset);
    int diff_count = target_entry - entry();

    if (diff_count > 0) {
      while (target_entry >= range_iter->upper) {
        data_iter += range_iter->upper - entry();
        range_iter++;
        entry_offset = 0;
      }

    } else {
      while (target_entry < range_iter->lower) {
        data_iter -= (entry() - range_iter->lower + 1); 
        // distance to prior range
        // [2, 3, 4) [6, 7, 8)
        //  ^     ^ <--- ^ 
        //  target, entry() - lower + 1 = 2 
        range_iter--;
        entry_offset = -1;
      }
    }

    auto offset = target_entry - entry();
    entry_offset += offset;
    data_iter += offset;
    META_INFO("current range {}, entry_offset {}, data {}", *range_iter, entry_offset, *data_iter);
    return *this;
  }

private:
  // make entry_offset positive
  void correct_entry_offset() {
    if (entry_offset < 0) { entry_offset = range_iter->upper + entry_offset; }
  }
};

}   // namespace MS

#endif