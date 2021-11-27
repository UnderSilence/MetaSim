
#ifndef METASIM_RANGE_SET_HPP
#define METASIM_RANGE_SET_HPP

#include <algorithm>
#include <ostream>
#include <tbb/tbb_stddef.h>
#include <vector>

// for MSVC's mom sakes
#undef max
#undef min


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
  RangeSet(const std::initializer_list<Range>& range_list) {
    for (auto& range : range_list) { merge(range); }
  }

  // tbb split construct function
  // Cutting RangeSet
  RangeSet(RangeSet& other, tbb::split);

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

  RangeSet& operator&=(const RangeSet& range_set) { return intersect(range_set), *this; }
  RangeSet& operator|=(const RangeSet& range_set) { return merge(range_set), *this; }
  RangeSet operator&(const RangeSet& range_set) { return RangeSet{*this} &= range_set; }
  RangeSet operator|(const RangeSet& range_set) { return RangeSet{*this} |= range_set; }

  void intersect() {}
  void intersect(const RangeSet& other_ranges);
  template<typename... Rest> void intersect(const Range& range, Rest&&... rest) {
    auto p = std::equal_range(ranges.begin(), ranges.end(), range);
    if (p.first != p.second) {
      auto leftmost_upper = p.first->upper;
      auto rightmost_lower = std::prev(p.second)->lower;
      std::vector<Range> new_ranges;
      if (std::distance(p.first, p.second) == 1) {
        new_ranges.push_back(range.intersect(*p.first));
      } else {
        new_ranges.push_back(range.intersect(*p.first));
        new_ranges.insert(new_ranges.end(), std::next(p.first), std::prev(p.second));
        new_ranges.push_back(range.intersect(*std::prev(p.second)));
      }
      std::swap(ranges, new_ranges);
    } else {
      // no intersections
      ranges.clear();
    }
    intersect(rest...);
  }


  void merge() {}
  void merge(const RangeSet& other_ranges);
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

  void erase(const Range& range);
};

inline std::ostream& operator<<(std::ostream& os, const Range& range) {
  os << "[" << range.lower << "," << range.upper << ")";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const RangeSet& set) {
  for (auto it = set.ranges.begin(); it != set.ranges.end(); ++it) { os << *it << " "; }
  return os;
}

}   // namespace MS

#endif