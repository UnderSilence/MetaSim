#ifndef METASIM_DATA_HPP
#define METASIM_DATA_HPP

#include "Core/data_array.hpp"
#include <functional>
#include <unordered_map>

namespace MS {

/* type handler */
template <typename T> struct TypeTag {
  std::string type_name;
  size_t type_hash;

  TypeTag(const char *type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}

  TypeTag(const std::string &type_name)
      : type_name(type_name), type_hash(std::hash<std::string>()(type_name)) {}
};

// template <typename... Types> class DataContainerIterator;
template <typename... Types> class DataContainerSubset;
// Dataset for manifolds attributes
class DataContainer {
public:
  // number of entry
  int total_size;
  std::unordered_map<size_t, std::unique_ptr<DataArrayBase>> dataset;

  template <typename Type>
  DataArray<Type> &append(const TypeTag<Type> &attr_tag,
                          const Interval &interval, const Type &init_value) {
    return append<Type>(attr_tag, interval,
                        std::vector<Type>(interval.length(), init_value));
  }

  template <typename Type>
  DataArray<Type> &get_array(const TypeTag<Type> &attr_tag) {
    auto iter = dataset.find(attr_tag.type_hash);
    return static_cast<DataArray<Type> &>(*iter->second);
  }

  template <typename Type>
  DataArray<Type> &append(const TypeTag<Type> &attr_tag,
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

  //  template <typename... Types>
  //  DataContainerIterator<Types...>
  //  SubsetIterator(const TypeTag<Types> &...tags) {
  //    return DataContainerIterator<Types...>(get_array(tags)...);
  //  }0

  template <typename... Types>
  DataContainerSubset<Types...> Subset(const TypeTag<Types> &...tags) {
    return DataContainerSubset<Types...>(get_array(tags)...);
  }

  template <typename... Types>
  DataContainerSubset<Types...> Subset(const Ranges &sub_ranges,
                                       const TypeTag<Types> &...tags) {
    return DataContainerSubset<Types...>(sub_ranges, get_array(tags)...);
  }
};

// template <typename Iter>
// using select_access_type_for = typename Iter::reference;
/// some template meta programming
template <typename... Args, std::size_t... Index>
auto any_match_impl(std::tuple<Args...> const &lhs,
                    std::tuple<Args...> const &rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... | (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template <typename... Args>
auto any_match(std::tuple<Args...> const &lhs, std::tuple<Args...> const &rhs)
    -> bool {
  return any_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}

template <typename... Args, std::size_t... Index>
auto all_match_impl(std::tuple<Args...> const &lhs,
                    std::tuple<Args...> const &rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... & (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template <typename... Args>
auto all_match(std::tuple<Args...> const &lhs, std::tuple<Args...> const &rhs)
    -> bool {
  return all_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}
///

template <typename... Types> class DataContainerSubset {
public:
  using array_set_type = std::tuple<DataArray<Types> &...>;
  using size_type = std::size_t;

  Ranges sub_ranges;
  array_set_type array_set;
  int begin_entry, end_entry;

  DataContainerSubset(DataArray<Types> &...array_set)
      : sub_ranges(array_set.ranges...), array_set(array_set...) {
    begin_entry = sub_ranges.cbegin()->lower;
    end_entry = begin_entry + sub_ranges.length();
  }

  DataContainerSubset(const Ranges &sub_ranges, DataArray<Types> &...array_set)
      : sub_ranges(sub_ranges, array_set.ranges...), array_set(array_set...) {
    begin_entry = sub_ranges.cbegin()->lower;
    end_entry = begin_entry + sub_ranges.length();
  }

  auto size() const { return sub_ranges.length(); }

  class iterator {
  public:
    using value_type = std::tuple<typename DataArray<Types>::reference...>;
    using iterator_set_type =
        std::tuple<typename DataArray<Types>::iterator...>;

    Ranges::const_iterator ranges_iter;
    Ranges::const_iterator ranges_end;
    Ranges::const_iterator ranges_begin;

    int entry_id;
    iterator_set_type iterators;
    iterator() = default;
    iterator(const iterator &) = default;
    ~iterator() = default;

    // data_id means data offset in data_array [0, size())
    iterator(DataContainerSubset &self, int data_id)
        : ranges_iter(self.sub_ranges.cbegin()),
          ranges_end(self.sub_ranges.cend()),
          ranges_begin(self.sub_ranges.cbegin()) {
      // initialize
      iterators = std::apply(
          [](auto &&...args) { return iterator_set_type(args.begin()...); },
          self.array_set);

      *this += data_id;
      // advance_entry(data_id); // get its entry_id first
      // advance_iterators_to(entry_id);
    }

    bool operator==(const iterator &rhs) const {
      return entry_id == rhs.entry_id && ranges_iter == rhs.ranges_iter &&
             all_match(iterators, rhs.iterators);
    }

    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
    auto operator*() -> value_type {
      return std::apply([](auto &&...args) { return value_type(*args...); },
                        iterators);
    }
    auto operator++() { return (*this += 1); }
    auto operator+=(int step) {
      // move entry_id by step
      advance_entry(step);
      advance_iterators_to(entry_id);
      return *this;
    }

    int advance_entry(int step) {
      while (entry_id + step >= ranges_iter->upper) {
        auto diff = ranges_iter->upper - entry_id;
        step -= diff;
        entry_id = (++ranges_iter)->lower;
      }
      return entry_id = entry_id + step;
    }

    void advance_iterators_to(int dst_entry_id) {
      std::apply(
          [&](auto &&...iters) { ((iters.advance_entry_to(dst_entry_id)), ...); },
          iterators);
      entry_id = dst_entry_id;
    }
  };

  using const_iterator = iterator;
  auto begin() { return iterator(sub_ranges.begin(), begin_entry, array_set); }
  auto begin() const {
    return const_iterator(sub_ranges.cbegin(), begin_entry, array_set);
  }
  auto end() { return iterator(sub_ranges.begin(), end_entry, array_set); }
  auto end() const {
    return const_iterator(sub_ranges.cbegin(), end_entry, array_set);
  }
};

// data zip iterator in common_ranges
// template <typename... Types> class DataContainerIterator {
// public:
//  using value_type = std::tuple<typename DataArray<Types>::reference...>;
//
//  std::tuple<DataArrayIterator<Types>...> iterators;
//  Ranges common_ranges;
//  Ranges::const_iterator ranges_iter;
//  // current entry cache
//  int entry_id;
//
//  DataContainerIterator(DataArray<Types> &...array_set)
//      : iterators(array_set.begin()...), common_ranges(array_set.ranges...),
//        ranges_iter(common_ranges.cbegin()), entry_id(0) {
//
//    entry_id = common_ranges.intervals.front().lower;
//    move_iterators(entry_id);
//  }
//
//  DataContainerIterator() = default;
//
//  DataContainerIterator<Types...> &operator++() { return (*this += 1); }
//  DataContainerIterator<Types...> &operator+=(int step) {
//    // move entry first
//    step_entry(step);
//    move_iterators(entry_id);
//    return *this;
//  }
//
//  bool operator==(const DataContainerIterator &rhs) const {
//    return all_match(iterators, rhs.iterators);
//  }
//
//  bool operator!=(const DataContainerIterator &rhs) const {
//    return !(*this == rhs);
//  }
//
//  int step_entry(int step) {
//    while (ranges_iter != common_ranges.cend() &&
//           entry_id + step > ranges_iter->upper) {
//      auto diff = ranges_iter->upper - entry_id;
//      step -= diff;
//      entry_id = (++ranges_iter)->lower;
//    }
//    return entry_id =
//               (ranges_iter != common_ranges.cend()) ? entry_id + step : -1;
//  }
//
//  auto operator*() -> value_type {
//    return std::apply([](auto &&...args) { return value_type(*args...); },
//                      iterators);
//  }
//
//  void move_iterators(int dst_entry_id) {
//    std::apply(
//        [&](auto &&...iters) { ((iters.move_iterator(dst_entry_id)), ...); },
//        iterators);
//  }
//};

} // namespace MS

#endif // METASIM_DATA_HPP
