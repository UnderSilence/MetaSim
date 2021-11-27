#include <algorithm>
#include <ostream>
#include <set>
#include <vector>
// #include <iostream>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <tbb/tbb_stddef.h>

#include "Core/range_set.hpp"

// for MSVC's mom sakes
#undef max
#undef min

namespace MS {

// A Range Set class with tbb::split enable

// tbb split construct function
// Cutting RangeSet
RangeSet::RangeSet(RangeSet& other, tbb::split)
  : lg2_grain_size(other.lg2_grain_size) {

  auto total_length = other.length();
  auto num_grains = total_length >> lg2_grain_size;
  auto split_length = (num_grains - (num_grains >> 1)) << lg2_grain_size;

  if (split_length >= total_length) {
    printf("[warning] skip splitting!");
    // no split, return in constructor?
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
      ranges.back().lower += curr_length - split_length;
      curr_range.upper = ranges.back().lower;
      if (curr_range.length() == 0) { ranges_to_split.pop_back(); }
      split_length = 0;
    }
  }

  std::reverse(ranges.begin(), ranges.end());

  // std::cout << "split to " << other.ranges << " with " << this->ranges << std::endl;

  assert(total_length == other.length() + this->length());
}

void RangeSet::intersect(const RangeSet& other_ranges) {
  auto p = begin();
  auto q = other_ranges.begin();
  std::vector<Range> new_ranges;

  while (p != end() && q != other_ranges.end()) {
    auto intersection = p->intersect(*q);
    if (intersection.length() > 0) { new_ranges.push_back(intersection); }
    p->upper < q->upper ? ++p : ++q;
  }
  std::swap(ranges, new_ranges);
}

void RangeSet::merge(const RangeSet& other_ranges) {
  for (auto iter = other_ranges.begin(); iter != other_ranges.end(); ++iter) { this->merge(*iter); }
}

void RangeSet::erase(const Range& range) {
  auto p = std::equal_range(ranges.begin(), ranges.end(), range);
  if (p.first != p.second) {
    auto new_lower = p.first->lower;
    auto new_upper = std::prev(p.second)->upper;
    p.second = ranges.erase(p.first, p.second);
    if (new_upper > range.upper) { p.second = ranges.insert(p.second, {range.upper, new_upper}); }
    if (new_lower < range.lower) { p.second = ranges.insert(p.second, {new_lower, range.lower}); }
  }
}

}   // namespace MS
