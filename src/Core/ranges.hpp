#ifndef METASIM_RANGES_HPP
#define METASIM_RANGES_HPP

#include <ostream>
#include <set>

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

  template<typename... Rest> 
  Ranges(const Interval& interval, Rest&&... rest) {
    merge(interval, std::forward<Rest>(rest)...);
  }

  std::set<Interval> intervals;

  auto cbegin() const { return intervals.cbegin(); }
  auto cend() const { return intervals.cend(); }
  auto begin() const { return intervals.begin(); }
  auto end() const { return intervals.end(); }
  auto size() const { return intervals.size(); }

  template <typename... Rest>
  void merge(const Interval &interval, const Rest &...rest) {
    auto p = intervals.equal_range(interval);
    if (p.first == p.second) {
      // equal ranges not found, merge directly
      intervals.insert(interval);
    } else {
      auto new_lower = std::min<int>(p.first->lower, interval.lower);
      auto new_upper =
          std::max<int>(std::prev(p.second)->upper, interval.upper);
      intervals.erase(p.first, p.second);
      intervals.insert({new_lower, new_upper});
    }
    merge(rest...);
  }
  // void merge(int lower, int upper) { merge({lower, upper}); }
  void merge() {}

  void erase(const Interval &interval) {
    auto p = intervals.equal_range(interval);
    if (p.first != p.second) {
      auto new_lower = p.first->lower;
      auto new_upper = std::prev(p.second)->upper;
      intervals.erase(p.first, p.second);
      if (new_lower < interval.lower) {
        intervals.insert({new_lower, interval.lower});
      }
      if (new_upper > interval.upper) {
        intervals.insert({interval.upper, new_upper});
      }
    }
  }
  // void erase(int lower, int upper) { erase({lower, upper}); }

  void intersect() {}
  void intersect(const Interval &interval) {
    auto p = intervals.equal_range(interval);
    if (p.first != p.second) {
      auto leftmost_upper = p.first->upper;
      auto rightmost_lower = std::prev(p.second)->lower;
      std::set<Interval> new_intervals;
      if (std::distance(p.first, p.second) == 1) {
        new_intervals.insert(interval.intersect(*p.first));
      } else {
        new_intervals.insert(std::next(p.first), std::prev(p.second));
        new_intervals.insert(interval.intersect(*p.first));
        new_intervals.insert(interval.intersect(*std::prev(p.second)));
      }
      std::swap(intervals, new_intervals);

    } else {
      // no intersections
      intervals.clear();
    }
  }

  template <typename... Rest>
  void intersect(const Ranges &other_ranges, const Rest &...rest) {
    auto p = begin(), q = other_ranges.begin();
    std::set<Interval> new_intervals;
    while (p != end() && q != other_ranges.end()) {
      auto intersection = p->intersect(*q);
      if (intersection.length() > 0) {
        new_intervals.insert(intersection);
      }
      p->upper < q->upper ? ++q : ++p;
    }
    std::swap(intervals, new_intervals);
    intersect(rest...);
  }

  template <typename... Rest>
  void merge(const Ranges &other_ranges, const Rest &...rest) {
    for (auto iter = other_ranges.begin(); iter != other_ranges.end(); ++iter) {
      this->merge(*iter);
    }
    merge(rest...);
  }
};

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