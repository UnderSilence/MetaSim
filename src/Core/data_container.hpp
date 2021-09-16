#ifndef METASIM_DATA_HPP
#define METASIM_DATA_HPP

#include "Core/data_array.hpp"
#include <functional>
#include <unordered_map>

namespace MS {

/* type handler */
template <typename T> struct AttributeTag {
  std::string type_name;
  size_t type_hash;

  AttributeTag(const char *type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}

  AttributeTag(const std::string &type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}
};

template <typename... Types> class DataContainerIterator;
// Dataset for manifolds attributes
class DataContainer {
public:
  // number of entry
  int total_size;
  std::unordered_map<size_t, std::unique_ptr<DataArrayBase>> dataset;

  template <typename Type>
  DataArray<Type> &append(const AttributeTag<Type> &attr_tag,
                          const Interval &interval, const Type &init_value) {
    return append<Type>(attr_tag, interval,
                        std::vector<Type>(interval.length(), init_value));
  }

  template <typename Type>
  DataArray<Type> &get_array(const AttributeTag<Type> &attr_tag) {
    auto iter = dataset.find(attr_tag.type_hash);
    return static_cast<DataArray<Type> &>(*iter->second);
  }

  template <typename Type>
  DataArray<Type> &append(const AttributeTag<Type> &attr_tag,
                          const Interval &interval, std::vector<Type> &&array) {
    total_size = std::max(interval.upper, total_size);
    auto iter = dataset.find(attr_tag.type_hash);

    if (iter == dataset.end()) {
      // if attribute array not found
      auto ret = dataset.emplace(
          attr_tag.type_hash,
          std::make_unique<DataArray<Type>>(
              attr_tag.type_name, Ranges(interval), std::move(array)));
      return static_cast<DataArray<Type> &>(*ret.first->second);
    } else {
      auto &old = static_cast<DataArray<Type> &>(*iter->second);
      old.ranges.merge(interval);
      old.array.insert(old.array.end(), std::make_move_iterator(array.begin()),
                       std::make_move_iterator(array.end()));
      // release array memory cause its empty now
      array.clear();
      return old;
    }
  }

  template <typename... Types>
  DataContainerIterator<Types...>
  ZipIterator(const AttributeTag<Types> &...tags) {
    return DataContainerIterator<Types...>(get_array(tags)...);
  }
};

// data zip iterator in common_ranges
template <typename... Types> class DataContainerIterator {
public:
  std::tuple<DataArrayIterator<Types>...> iterators;
  Ranges common_ranges;
  Ranges::const_iterator ranges_iter;
  // current entry&value index
  int entry_id;


  DataContainerIterator(DataArray<Types> &...array_set)
      : iterators(array_set.array.begin()...),
        common_ranges(array_set.ranges...),
        ranges_iter(common_ranges.cbegin()),entry_id(0) {}

  DataContainerIterator() = default;

  std::tuple<Types...> base() const;
};

} // namespace MS

#endif // METASIM_DATA_HPP
