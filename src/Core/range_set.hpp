
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
  size_t _total_length{0};

  static RangeSet All;
  static RangeSet Empty;

  using iterator = std::vector<Range>::iterator;
  using const_iterator = std::vector<Range>::const_iterator;

  RangeSet() = default;
  RangeSet(const RangeSet& ranges) = default;
  template<typename... Rest>
  RangeSet(const RangeSet& other, Rest&&... rest)
    : ranges(other.ranges)
    , lg2_grain_size(other.lg2_grain_size) {
    (intersect(rest), ...);
  }
  RangeSet(const std::initializer_list<Range>& range_list) {
    for (auto& range : range_list) {
      merge(range);
    }
  }
  RangeSet(const Range& range)
    : ranges({range}) {}

  // tbb split construct function
  // Cutting RangeSet
  RangeSet(RangeSet& other, tbb::split);

  // need to scan the array in O(N), avoid use it frequently.
  // TODO: add a lazy tag for updating the array
  int length() const {
    // hahahh
    int result = 0;
    for (auto& range : ranges) {
      result += range.length();
    }
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

  auto& front() { return ranges.front(); }
  auto& back() { return ranges.back(); }

  // Type Range or Type RangeSet
  template<typename TRorTRS>
  RangeSet& operator&=(TRorTRS&& range_set) {
    return intersect(range_set), *this;
  }
  template<typename TRorTRS>
  RangeSet& operator|=(TRorTRS&& range_set) {
    return merge(range_set), *this;
  }

  /*
   *
   * [1, 3) [5, 8) query x = 9
   * return offset == 5 = 2 + 3
   *
   */
  auto query_offset(int x) const {
    auto offset = 0;
    for (const auto& range : ranges) {
      if (x > range.upper) {
        offset += range.length();
      } else {
        offset += x - range.lower;
        break;
      }
    }
    return offset;
  }

  void intersect() {}
  // slow sometimes
  void intersect(const RangeSet& other_ranges);
  // use binary search speedups
  template<typename... Rest>
  void intersect(const Range& range, Rest&&... rest) {
    auto p = std::equal_range(ranges.begin(), ranges.end(), range);
    RangeSet new_range_set(*this);
    auto& new_ranges = new_range_set.ranges;

    if (p.first != p.second) {
      auto leftmost_upper = p.first->upper;
      auto rightmost_lower = std::prev(p.second)->lower;
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
  }

  // operator with single range could be faster
  void merge() {}
  void merge(const RangeSet& other_ranges);
  template<typename... Rest>
  void merge(const Range& range, Rest&&... rest) {
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
    return merge(rest...);
  }

  void erase(const Range& range);
};

template<typename TRorTRS>
inline RangeSet operator&(RangeSet lhs, TRorTRS&& rhs) {
  lhs &= rhs;
  return lhs;
}

template<typename TRorTRS>
inline RangeSet operator|(RangeSet lhs, const TRorTRS& rhs) {
  lhs |= rhs;
  return lhs;
}


inline std::ostream& operator<<(std::ostream& os, const Range& range) {
  os << "[" << range.lower << "," << range.upper << ")";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const RangeSet& set) {
  for (auto it = set.ranges.begin(); it != set.ranges.end(); ++it) {
    os << *it << " ";
  }
  return os;
}

}   // namespace MS

#endif