#include <tpf_chrono_random.hpp>
#include <tpf_output.hpp>
#include <tpf_parallel.hpp>


#include <tbb/tbb.h>
#include <tbb/tbbmalloc_proxy.h>


// using namespace std

/*
    I hope you learned how to read, interpret, analyze source code written by others.

    Also, how to create generic algorithm.

    Now, I will explain the concept of "Policy" in C++ Programming
*/

namespace chr = tpf::chrono_random;

tpf::sstream stream;
auto&        endl = tpf::endl;
auto&        endL = tpf::endL;

using mutex_t = tbb::mutex;
mutex_t mutex;

namespace tpf {
template<typename IteratorType, typename ValueType = std::iterator_traits<IteratorType>::value_type,
         typename PredicateType =
           std::less<ValueType>>   // parameter that is used for operation is called "Policy"
auto partition(IteratorType b, IteratorType e, PredicateType&& predicate = PredicateType{}) {
  auto i = b, j = e;
  std::advance(j, -1);

  auto pivot_value = *b;

  while (i != j) {

    while (i != j && predicate(pivot_value, *j)) --j;

    while (i != j && (*i == pivot_value || predicate(*i, pivot_value))) ++i;

    std::iter_swap(i, j);
  }

  std::iter_swap(b, i);

  return i;
}

template<typename IteratorType, typename ValueType = std::iterator_traits<IteratorType>::value_type,
         typename PredicateType = std::less<ValueType>>
void serial_quick_sort(IteratorType b, IteratorType e,
                       PredicateType&& predicate = PredicateType{}) {
  if (b == e) return;

  auto i = tpf::partition(b, e, predicate);

  serial_quick_sort(b, i, predicate);

  std::advance(i, 1);
  serial_quick_sort(i, e, predicate);
}

template<typename IteratorType, typename ValueType = std::iterator_traits<IteratorType>::value_type,
         typename PredicateType = std::less<ValueType>>
class SplitRange {

public:
  // using IteratorType = std::list<double>::iterator;
  // using PredicateType = std::less<double>;

private:
  IteratorType         m_begin;
  IteratorType         m_end;
  const PredicateType& m_predicate;

  // pay attention to "grain size"
  const size_t m_grain_size;

public:
  SplitRange(IteratorType begin, IteratorType end, const PredicateType& predicate = PredicateType{},
             size_t grain_size = 2)
    : m_begin{begin}
    , m_end{end}
    , m_predicate{predicate}
    , m_grain_size{grain_size} {}

  SplitRange(const SplitRange&) = default;
  ~SplitRange()                 = default;

  // this member is REQUIRED
  bool empty() const { return this->m_begin == this->m_end; }

  // this member is REQUIRED
  bool is_divisible() const {
    auto count = std::distance(this->m_begin, this->m_end);

    // So, our SplitRange is divible
    //  if element count is greater than grain size
    return count > this->m_grain_size;
  }

  // split constructor is REQUIRED
  SplitRange(SplitRange& range_to_be_splitted, tbb::split)
    : m_begin{range_to_be_splitted.m_begin}
    , m_end{range_to_be_splitted.m_end}
    , m_predicate{range_to_be_splitted.m_predicate}
    , m_grain_size{range_to_be_splitted.m_grain_size} {

    // 2D3D-#045 - How to Read Scientific/Technical Books Efficiently and Effectively
    // https://www.youtube.com/watch?v=Ntzi0I1XenQ&list=PL1_C6uWTeBDHMqioaL1yQ_B0fi7uzxIPB&index=49
    auto i = tpf::partition(m_begin, m_end, m_predicate);

    this->m_end = i;

    std::advance(i, 1);

    range_to_be_splitted.m_begin = i;
  }

  // this is our own utility function
  size_t chunk_size() const { return (size_t)std::distance(this->m_begin, this->m_end); }

  // this is for our own use
  const PredicateType& predicate() const { return this->m_predicate; }

  const IteratorType& begin() const { return this->m_begin; }

  const IteratorType& end() const { return this->m_end; }

};   // end of class SplitRange

template<typename IteratorType, typename ValueType = std::iterator_traits<IteratorType>::value_type,
         typename PredicateType = std::less<ValueType>>
void parallel_quick_sort(IteratorType begin, IteratorType end,
                         PredicateType&& predicate = PredicateType{}) {
  auto sort_body = [](const auto& range) {
    {
      mutex_t::scoped_lock lock{mutex};
      // stream << "Type of range: " << Tpf_GetTypeCategory(range) << endl;

      stream << "chunk size: " << range.chunk_size() << endl;
    }

    if (range.chunk_size() > 1)
      tpf::serial_quick_sort(range.begin(), range.end(), range.predicate());
  };

  tbb::parallel_for(SplitRange{begin, end, predicate, 10}, sort_body, tbb::simple_partitioner{});
}

}   // end of namespace tpf

void test_serial_quicksort() {
  using element_t   = double;
  using container_t = std::list<element_t>;

  container_t cntr{3, 1, 4, 2, 5};

  stream << "Before Sort: " << cntr << endL;

  tpf::serial_quick_sort(cntr.begin(), cntr.end());
  // tpf::serial_quick_sort(cntr.begin(), cntr.end(), std::greater<element_t>{});

  stream << "After Sort: " << cntr << endL;
}

void test_parallel_quicksort() {
  using element_t   = double;
  using container_t = std::list<element_t>;

  container_t cntr{3, 1, 4, 2, 5};

  size_t problem_size = cntr.size();

  stream << "Problem size: " << problem_size << endl;
  stream << "Before Sort: " << cntr << endl;

  tpf::parallel_quick_sort(cntr.begin(), cntr.end(), std::greater<element_t>{});

  stream << "After Sort: " << cntr << endl;
}

void experiment_parallel_quicksort() {
  using element_t   = double;
  using container_t = std::vector<element_t>;

  size_t problem_size = 1000;

  container_t cntr(problem_size);

  std::iota(cntr.begin(), cntr.end(), element_t{1});

  std::shuffle(cntr.begin(), cntr.end(), std::mt19937{chr::seed()});

  stream << "Before Sort: " << cntr << endL;

  tpf::parallel_quick_sort(cntr.begin(), cntr.end());

  stream << "After Sort: " << cntr << endl;
}

int main() {
  // test_serial_quicksort();

  // test_parallel_quicksort();

  experiment_parallel_quicksort();
}