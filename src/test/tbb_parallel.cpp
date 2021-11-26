
#include <bits/stdc++.h>
#include <tbb/parallel_for.h>
#include <tbb/tick_count.h>

#include "Core/range_set.hpp"
using namespace MS;

int main() {

  RangeSet test_set;
  test_set.merge({0, 200});
  test_set.merge({300, 800});

  auto test_body = [](const auto& range_set) { std::cout << range_set << std::endl; };

  tbb::parallel_for(test_set, test_body);


  // auto t0 = tbb::tick_count::now();

  // std::vector<double> values(10000000);
  // tbb::parallel_for(tbb::blocked_range<int>(0, values.size()), [&](tbb::blocked_range<int> r) {
  //   for (int i = r.begin(); i < r.end(); ++i) { values[i] = std::sin(i * 0.001); }
  // });

  // auto t1 = tbb::tick_count::now();
  // printf("work took %g seconds\n", (t1 - t0).seconds());
  return 0;
}