
#ifndef METASIM_RANGE_SET_HPP
#define METASIM_RANGE_SET_HPP

#include <algorithm>
#include <ostream>
#include <set>
#include <vector>
// #include <iostream>
#include <cassert>
#include <numeric>
#include <tbb/tbb_stddef.h>



namespace MS {
struct Range {
  // [lower, upper)
  int lower, upper;
  bool operator<(const Range& rhs) const { return this->upper <= rhs.lower; }
  bool operator==(const Range& rhs) const { return lower == rhs.lower && upper == rhs.upper; }

  Range intersect(const Range& rhs) const {
    return {std::max(lower, rhs.lower), std::min(upper, rhs.upper)};
  }
  int length() const { return upper - lower; }
};

// A Range Set class with tbb::split enable
struct RangeSet {
  std::vector<Range> ranges;
  // log2 of grain size, set default to 2^7 = 128
  size_t lg2_grain_size{7};

  using iterator = std::vector<Range>::iterator;
  using const_iterator = std::vector<Range>::const_iterator;

  RangeSet() = default;
  RangeSet(const RangeSet& ranges) = default;
  template<typename... Rest>
  RangeSet(const RangeSet& other, const Rest&... rest)
    : ranges(other.ranges)
    , lg2_grain_size(other.lg2_grain_size) {
    intersect(rest...);
  }

  // tbb split construct function
  // Cutting RangeSet
  RangeSet(RangeSet& other, tbb::split)
    : lg2_grain_size(other.lg2_grain_size) {

    auto total_length = other.length();
    auto num_grains = total_length >> lg2_grain_size;
    auto split_length = (num_grains - (num_grains >> 1)) << lg2_grain_size;

    if (split_length >= total_length) {
      // no split
      return;
    }

    auto& ranges_to_split = other.ranges;
    while (!ranges_to_split.empty() && split_length) {
      auto& curr_range = ranges_to_split.back();
      auto curr_length = curr_range.length();
      ranges.push_back(curr_range);

      if (split_length > curr_length) {
        ranges_to_split.pop_back();
        split_length -= curr_length;
      } else {
        auto diff = curr_length - split_length;
        ranges.back().lower += diff;
        curr_range.upper -= diff;
        if (curr_range.length() == 0) { ranges_to_split.pop_back(); }
        split_length = 0;
      }
    }

    std::reverse(ranges.begin(), ranges.end());
    assert(total_length == other.length() + this->length());
  }

  int length() const {
    // hahahh
    int result = 0;
    for (auto& range : ranges) { result += range.length(); }
    return result;
  }

  bool empty() const { return length() == 0; }
  // tbb divisible function
  bool is_divisible() const { return length() > (1 << lg2_grain_size); };
  Range& operator[](size_t i) { return ranges[i]; }
  const Range& operator[](size_t i) const { return ranges[i]; }

  auto cbegin() const { return ranges.cbegin(); }
  auto cend() const { return ranges.cend(); }
  auto begin() { return ranges.begin(); }
  auto end() { return ranges.end(); }
  auto begin() const { return ranges.cbegin(); }
  auto end() const { return ranges.cend(); }
  auto rbegin() { return ranges.rbegin(); }
  auto rend() { return ranges.rend(); }
  auto rbegin() const { return ranges.crbegin(); }
  auto rend() const { return ranges.crend(); }

  void intersect() {}
  template<typename... Rest> void intersect(const RangeSet& other_ranges, Rest&&... rest) {
    auto p = begin();
    auto q = other_ranges.begin();
    std::vector<Range> new_ranges;

    while (p != end() && q != other_ranges.end()) {
      auto intersection = p->intersect(*q);
      if (intersection.length() > 0) { new_ranges.push_back(intersection); }
      p->upper < q->upper ? ++p : ++q;
    }
    std::swap(ranges, new_ranges);
    intersect(rest...);
  }

  void merge() {}
  template<typename... Rest> void merge(const Range& range, Rest&&... rest) {
    auto p = std::equal_range(ranges.begin(), ranges.end(), range);
    if (p.first == p.second) {
      // equal ranges not found, merge directly
      ranges.insert(p.second, range);
    } else {
      auto new_lower = std::min<int>(p.first->lower, range.lower);
      auto new_upper = std::max<int>(std::prev(p.second)->upper, range.upper);
      p.second = ranges.erase(p.first, p.second);
      ranges.insert(p.second, {new_lower, new_upper});
    }
    merge(rest...);
  }
  template<typename... Rest> void merge(const RangeSet& other_ranges, Rest&&... rest) {
    for (auto iter = other_ranges.begin(); iter != other_ranges.end(); ++iter) {
      this->merge(*iter);
    }
    merge(rest...);
  }
};

inline std::ostream& operator<<(std::ostream& os, const Range& interval) {
  os << "[" << interval.lower << "," << interval.upper << ")";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const RangeSet& set) {
  for (auto it = set.ranges.begin(); it != set.ranges.end(); ++it) { os << *it << " "; }
  return os;
}

}   // namespace MS

#endif