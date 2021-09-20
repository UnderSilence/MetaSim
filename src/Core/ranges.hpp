#ifndef METASIM_RANGES_HPP
#define METASIM_RANGES_HPP

#include <ostream>
#include <set>
#include <vector>

// #include <iostream>
#include <numeric>

namespace MS {

// disjoint ranges
struct Ranges {
public:
  struct Interval {
    // [lower, upper)
    int lower, upper;
    bool operator<(const Interval &rhs) const {
      return this->upper <= rhs.lower;
    }
    bool operator==(const Interval &rhs) const {
      return lower == rhs.lower && upper == rhs.upper;
    }

    Interval intersect(const Interval &rhs) const {
      return {std::max(lower, rhs.lower), std::min(upper, rhs.upper)};
    }
    int length() const { return upper - lower; }
  };

  // merge constructor
  Ranges() = default;
  Ranges(const Ranges &ranges) = default;
  template <typename... Rest>
  Ranges(const Ranges &ranges, const Rest &...rest) {
    merge(ranges);
    intersect(rest...);
  }

  template <typename... Rest>
  Ranges(const Interval &interval, const Rest &...rest) {
    merge(interval, rest...);
  }

  std::vector<Interval> intervals;
  using iterator = std::vector<Interval>::iterator;
  using const_iterator = std::vector<Interval>::const_iterator;

  auto cbegin() const { return intervals.cbegin(); }
  auto cend() const { return intervals.cend(); }

  auto begin() { return intervals.begin(); }
  auto end() { return intervals.end(); }
  auto begin() const { return intervals.cbegin(); }
  auto end() const { return intervals.cend(); }

  auto size() const { return intervals.size(); }
  // calculate ranges length sum between [lower, upper)
  auto length(int lower, int upper) const {
    Interval interval{lower, upper};
    auto p = std::equal_range(intervals.begin(), intervals.end(), interval);
    auto result = 0;
    for (auto iter = p.first; iter != p.second; iter++) {
      if (lower > iter->lower) {
        result += iter->upper - lower;
      } else if (upper < iter->upper) {
        result += upper - iter->lower;
      } else {
        result += iter->length();
      }
    }
    return result;
  }

  auto length() const {
    return std::accumulate(
        intervals.cbegin(), intervals.cend(), 0,
        [](int result, auto &&rhs) -> auto { return result + rhs.length(); });
  }

  template <typename... Rest>
  void merge(const Interval &interval, Rest &&...rest) {
    auto p = std::equal_range(intervals.begin(), intervals.end(), interval);
    if (p.first == p.second) {
      // equal ranges not found, merge directly
      intervals.insert(p.second, interval);
    } else {
      auto new_lower = std::min<int>(p.first->lower, interval.lower);
      auto new_upper =
          std::max<int>(std::prev(p.second)->upper, interval.upper);
      p.second = intervals.erase(p.first, p.second);
      intervals.insert(p.second, {new_lower, new_upper});
    }
    merge(rest...);
  }
  // void merge(int lower, int upper) { merge({lower, upper}); }
  void merge() {}

  void erase(const Interval &interval) {
    auto p = std::equal_range(intervals.begin(), intervals.end(), interval);
    if (p.first != p.second) {
      auto new_lower = p.first->lower;
      auto new_upper = std::prev(p.second)->upper;
      p.second = intervals.erase(p.first, p.second);
      if (new_upper > interval.upper) {
        p.second = intervals.insert(p.second, {interval.upper, new_upper});
      }
      if (new_lower < interval.lower) {
        p.second = intervals.insert(p.second, {new_lower, interval.lower});
      }
    }
  }
  // void erase(int lower, int upper) { erase({lower, upper}); }

  void intersect() {}
  /* // not needed
  void intersect(const Interval &interval) {
    auto p = std::equal_range(intervals.begin(), intervals.end(), interval);
    if (p.first != p.second) {
      auto leftmost_upper = p.first->upper;
      auto rightmost_lower = std::prev(p.second)->lower;
      std::vector<Interval> new_intervals;
      if (std::distance(p.first, p.second) == 1) {
        new_intervals.push_back(interval.intersect(*p.first));
      } else {
        new_intervals.push_back(interval.intersect(*p.first));
        new_intervals.insert(new_intervals.end(), std::next(p.first),
                             std::prev(p.second));
        new_intervals.push_back(interval.intersect(*std::prev(p.second)));
      }
      std::swap(intervals, new_intervals);

    } else {
      // no intersections
      intervals.clear();
    }
  }
  */

  template <typename... Rest>
  void intersect(const Ranges &other_ranges, Rest &&...rest) {
    auto p = begin();
    auto q = other_ranges.begin();
    std::vector<Interval> new_intervals;
    while (p != end() && q != other_ranges.end()) {
      auto intersection = p->intersect(*q);
      if (intersection.length() > 0) {
        new_intervals.push_back(intersection);
      }
      p->upper < q->upper ? ++p : ++q;
    }
    std::swap(intervals, new_intervals);
    intersect(rest...);
  }

  template <typename... Rest>
  void merge(const Ranges &other_ranges, Rest &&...rest) {
    for (auto iter = other_ranges.begin(); iter != other_ranges.end(); ++iter) {
      this->merge(*iter);
    }
    merge(rest...);
  }
};

using Interval = Ranges::Interval;

inline std::ostream &operator<<(std::ostream &os,
                                const Ranges::Interval &interval) {
  os << "[" << interval.lower << "," << interval.upper << ")";
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const Ranges &ranges) {
  for (auto it = ranges.intervals.begin(); it != ranges.intervals.end(); ++it) {
    os << *it << " ";
  }
  return os;
}

} // namespace MS

#endif