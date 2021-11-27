
#include "Core/range_set.hpp"
#include <iostream>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>
#include <tbb/tick_count.h>

using namespace MS;

int main() {

  RangeSet test_set;
  test_set.merge({0, 128});
  // test_set.merge({300, 800});
  // test_set.intersect({{100, 500}, {600, 700}});

  std::cout << "curr test_set:" << test_set << std::endl;

  test_set.lg2_grain_size = 7;

  auto test_body = [](const auto& range_set) {
      tbb::mutex::scoped_lock();
      std::cout << range_set.length() << std::endl;
  };

  tbb::parallel_for(test_set, test_body);


  std::vector<int> x{1, 3, 4, 5};
  auto it = x.end();
  printf("test end %d\n", *it);

  // auto t0 = tbb::tick_count::now();

  // std::vector<double> values(10000000);
  // tbb::parallel_for(tbb::blocked_range<int>(0, values.size()), [&](tbb::blocked_range<int> r) {
  //   for (int i = r.begin(); i < r.end(); ++i) { values[i] = std::sin(i * 0.001); }
  // });

  // auto t1 = tbb::tick_count::now();
  // printf("work took %g seconds\n", (t1 - t0).seconds());
  return 0;
}