module;

#include <array>
#include <cstddef>
#include <utility>

export module Goom.Utils:ArrayUtils;

namespace GOOM::UTILS
{

namespace DETAIL
{

template<typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> CreateArrayImpl(
    const T value, [[maybe_unused]] std::index_sequence<Is...> args)
{
  // Cast 'Is' to void to remove the warning: unused value
  return {{(static_cast<void>(Is), value)...}};
}

} // namespace DETAIL

export template<typename T, std::size_t N>
constexpr std::array<T, N> CreateArray(const T& value)
{
  return DETAIL::CreateArrayImpl(value, std::make_index_sequence<N>());
}

} // namespace GOOM::UTILS
