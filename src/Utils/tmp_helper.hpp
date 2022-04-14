#ifndef _METASIM_TMP_HELPER_HPP_
#define _METASIM_TMP_HELPER_HPP_


// template <typename Iter>
// using select_access_type_for = typename Iter::reference;
/// some template meta programming
template<typename... Args, std::size_t... Index>
auto any_match_impl(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... | (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template<typename... Args>
auto any_match(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs) -> bool {
  return any_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}

template<typename... Args, std::size_t... Index>
auto all_match_impl(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs,
                    std::index_sequence<Index...>) -> bool {
  auto result = false;
  result = (... & (std::get<Index>(lhs) == std::get<Index>(rhs)));
  return result;
}

template<typename... Args>
auto all_match(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs) -> bool {
  return all_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}
///

#endif   //_METASIM_TMP_HELPER_HPP_