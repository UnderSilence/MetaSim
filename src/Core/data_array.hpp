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
class DataArrayIterator {};

} // namespace MS

#endif