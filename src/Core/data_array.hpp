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

// entry_index -> data_index -> data
template <typename Type> class DataArray : public DataArrayBase {
public:
  using DataArrayBase::name;
  using DataArrayBase::ranges;
  std::vector<Type> array;

  DataArray(const std::string &name, const Ranges &ranges, std::vector<Type> &&array)
      : DataArrayBase(name, ranges), array(std::move(array)) {}
};

// RandomAccessIterator
template<typename Type>
class DataArrayIterator {
public:
  Ranges sub_ranges;
  typename std::vector<Type>::iterator iter;
  int entry_id, value_id;
  DataArrayIterator(typename std::vector<Type>::iterator iter) : iter(iter) {

  }
};

} // namespace MS

#endif