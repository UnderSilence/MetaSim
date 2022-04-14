
#include "Core/range_set.hpp"
#include <iostream>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>
#include <tbb/tick_count.h>

using namespace MS;

int main() {

  RangeSet test_set;
  test_set.merge({0, 1000});
  // test_set.merge({300, 800});
  // test_set.intersect({{100, 500}, {600, 700}});

  std::cout << "curr test_set:" << test_set << std::endl;

  test_set.lg2_grain_size = 7;

  auto test_body = [](const auto& range_set) {
    tbb::mutex::scoped_lock();
    std::cout << range_set << "\n";
  };

  tbb::parallel_for(test_set, test_body);
  return 0;
}