#ifndef METASIM_DATA_HPP
#define METASIM_DATA_HPP

#include "core/data_array.hpp"
#include "utils/logger.hpp"
#include <functional>
#include <unordered_map>


namespace MS {

/* type handler */
template<typename T>
struct TypeTag {
  std::string type_name;
  size_t type_hash;

  TypeTag(const char* type_name)
    : type_name(type_name)
    , type_hash(std::hash<std::string>()(type_name)) {}

  TypeTag(const std::string& type_name)
    : type_name(type_name)
    , type_hash(std::hash<std::string>()(type_name)) {}
};

// template <typename... Types> class DataContainerIterator;
template<typename... Types>
class DataSubset;
// Dataset for manifolds attributes
class DataContainer {
public:
  // number of entry
  int total_size;
  std::unordered_map<size_t, std::unique_ptr<DataArrayBase>> dataset;

  template<typename Type>
  DataArray<Type>& append(const TypeTag<Type>& attr_tag, const Range& range,
                          const Type& init_value) {
    return append<Type>(attr_tag, range, std::vector<Type>(range.length(), init_value));
  }

  template<typename Type>
  DataArray<Type>& get_array(const TypeTag<Type>& attr_tag) {
    auto iter = dataset.find(attr_tag.type_hash);
    return static_cast<DataArray<Type>&>(*iter->second);
  }

  template<typename Type>
  DataArray<Type>& append(const TypeTag<Type>& attr_tag, const Range& range,
                          std::vector<Type>&& array) {
    total_size = std::max(range.upper, total_size);
    auto iter = dataset.find(attr_tag.type_hash);

    if (iter == dataset.end()) {
      // if attribute array not found
      auto ret = dataset.emplace(
        attr_tag.type_hash,
        std::make_unique<DataArray<Type>>(attr_tag.type_name, RangeSet{range}, std::move(array)));
      return static_cast<DataArray<Type>&>(*ret.first->second);
    } else {
      // update data_array with range
      auto& old = static_cast<DataArray<Type>&>(*iter->second);

      // @TODO new range must at end of ranges
      old.ranges.merge(range);
      old.array.insert(old.array.end(),
                       std::make_move_iterator(array.begin()),
                       std::make_move_iterator(array.end()));
      // release array memory cause its empty now
      array.resize(0);
      return old;
    }
  }

  //  template <typename... Types>
  //  DataContainerIterator<Types...>
  //  SubsetIterator(const TypeTag<Types> &...tags) {
  //    return DataContainerIterator<Types...>(get_array(tags)...);
  //  }

  template<typename... Types>
  DataSubset<Types...> Subset(const TypeTag<Types>&... tags) {
    return DataSubset<Types...>(get_array(tags)...);
  }

  template<typename... Types>
  DataSubset<Types...> Subset(const RangeSet& sub_ranges, const TypeTag<Types>&... tags) {
    return DataSubset<Types...>(sub_ranges, get_array(tags)...);
  }
};

// template <typename Iter>
// using select_access_type_for = typename Iter::reference;
/// some template meta programming
template<typename... Args, std::size_t... Index>
auto any_match_impl(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... | (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template<typename... Args>
auto any_match(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs) -> bool {
  return any_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}

template<typename... Args, std::size_t... Index>
auto all_match_impl(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... & (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template<typename... Args>
auto all_match(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs) -> bool {
  return all_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}
///

// Data subset accessor, can't edit
template<typename... Types>
class DataSubset {
public:
  using array_set_type = std::tuple<DataArray<Types>&...>;
  using size_type = std::size_t;

  // array_set's common ranges by default
  RangeSet sub_ranges;
  // each array set contains its own ranges;
  array_set_type array_set;
  // std::vector<size_t> entry2index;
  // std::vector<size_t> index2entry;

  DataSubset() = default;

  DataSubset(DataArray<Types>&... array_set)
    : sub_ranges(array_set.ranges...)
    , array_set(array_set...) {}

  DataSubset(const RangeSet& sub_ranges, DataArray<Types>&... array_set)
    : sub_ranges(sub_ranges, array_set.ranges...)
    , array_set(array_set...) {}

  // shrink
  DataSubset(const DataSubset& other) = default;

  DataSubset(DataSubset& other, tbb::split)
    : sub_ranges(other.sub_ranges, tbb::split{})
    , array_set(other.array_set) {}

  bool is_divisible() const { return sub_ranges.is_divisible(); }
  bool empty() const { return sub_ranges.empty(); }
  // split data_subset
  auto size() const { return sub_ranges.length(); }
  auto begin() { return iterator(*this, 0); }
  auto begin() const { return const_iterator(*this, 0); }
  // -1 set to fake ends, cause size() is costy
  auto end() { return iterator(*this, -1); }
  auto end() const { return const_iterator(*this, -1); }

  // https://stackoverflow.com/questions/43277513/c-iterator-with-hasnext-and-next
  // https://stackoverflow.com/questions/7758580/writing-your-own-stl-container/7759622#7759622
  // https://softwareengineering.stackexchange.com/questions/212344/is-it-bad-practice-to-make-an-iterator-that-is-aware-of-its-own-end

  // 很难去完整的定义标准的 iterator 流程，不如直接抽象集合操作
  // iterators forward increment 操作过于昂贵，定义迭代器操作复杂
  // 可能的实现方式，参考 std::set, std::unordered_map
  template<class Op>
  void foreach_element(Op op) {
    using iterator_set_type = std::tuple<typename DataArray<Types>::iterator...>;
    using value_type = std::tuple<typename DataArray<Types>::reference...>;
    iterator_set_type value_iters =
      std::apply([](auto&&... args) { return iterator_set_type(args.begin()...); }, array_set);

    for (auto iter = sub_ranges.begin(); iter != sub_ranges.end(); ++iter) {
      for (auto entry = iter->lower; entry < iter->upper; ++entry) {
        std::apply([&](auto&&... iters) { ((iters.advance_to(entry)), ...); }, value_iters);
        std::apply([&](auto&&... args) { return op(*args...); }, value_iters);
      }
    }
  }

  class iterator {
  public:
    using value_type = std::tuple<typename DataArray<Types>::reference...>;
    using iterator_set_type = std::tuple<typename DataArray<Types>::iterator...>;
    using ranges_iterator = RangeSet::iterator;
    using iterator_category = std::forward_iterator_tag;

    int entry_id;
    // TODO: iterators knowing its end, bad implementation?
    // Maybe a range-based abstract is OK. e.g. foreach_element
    int entry_begin, entry_end;   // entry_begin used for bidirection
    ranges_iterator ranges_iter;
    iterator_set_type value_iters;

    iterator() = default;
    iterator(const iterator&) = default;
    ~iterator() = default;

    // data_id means data offset in data_array [0, size())
    iterator(DataSubset& self, size_type index)
      : entry_id(0) {

      ranges_iter = self.sub_ranges.begin();
      value_iters = std::apply([](auto&&... args) { return iterator_set_type(args.begin()...); },
                               self.array_set);

      if (!self.sub_ranges.empty()) {
        entry_begin = self.sub_ranges.front().lower;
        entry_end = self.sub_ranges.back().upper;
        entry_id = entry_begin;
      } else {
        entry_begin = entry_end = entry_id;
      }
      *this += index;
    }

    bool operator==(const iterator& rhs) const { return entry_id == rhs.entry_id; }
    bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

    auto operator*() -> value_type {
      return std::apply([](auto&&... args) { return value_type(*args...); }, value_iters);
    }

    // move entry_id by step
    auto operator++() { return (*this += 1); }
    auto operator--() { return (*this -= 1); }
    auto operator-=(int step) { return *this += -step; }
    auto operator+=(int step) { return safe_advance(step), *this; }

    // What if stepping outside of the boundary?
    int safe_advance(int step) {
      // pass over end? check it first (dirty impl.)
      if (entry_id + step < entry_begin || entry_id + step >= entry_end) {
        // directly return iterator without adjust iterator
        return entry_id = entry_end;
      }

      // adjust iterator
      // while (entry_id >= ranges_iter->upper && ranges_iter->upper != entry_end)
      //   ++ranges_iter;
      // while (entry_id < ranges_iter->lower && ranges_iter->lower != entry_begin)
      //   --ranges_iter;

      for (; entry_id + step >= ranges_iter->upper;) {
        auto diff = ranges_iter->upper - entry_id;
        step -= diff;
        entry_id = (++ranges_iter)->lower;
      }

      // for (; entry_id + step < ranges_iter->lower;) {
      //   auto diff = entry_id - ranges_iter->lower + 1;
      //   step -= diff;
      //   entry_id = (--ranges_iter)->upper - 1;
      // }

      entry_id += step;
      std::apply([&](auto&&... iters) { ((iters.advance_to(entry_id)), ...); }, value_iters);
      META_INFO("current entryid = {}", entry_id);
      return entry_id;
    }
  };
  using const_iterator = iterator;
};

// data zip iterator in common_ranges
// template <typename... Types> class DataContainerIterator {
// public:
//  using value_type = std::tuple<typename DataArray<Types>::reference...>;
//
//  std::tuple<DataArrayIterator<Types>...> iterators;
//  RangeSet common_ranges;
//  RangeSet::const_iterator ranges_iter;
//  // current entry cache
//  int entry_id;
//
//  DataContainerIterator(DataArray<Types> &...array_set)
//      : iterators(array_set.begin()...), common_ranges(array_set.ranges...),
//        ranges_iter(common_ranges.cbegin()), entry_id(0) {
//
//    entry_id = common_ranges.ranges.front().lower;
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

}   // namespace MS

#endif   // METASIM_DATA_HPP
