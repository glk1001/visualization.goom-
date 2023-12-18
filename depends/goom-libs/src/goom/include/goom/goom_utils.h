#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

namespace GOOM
{

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

[[nodiscard]] auto GetGoomLibCompilerVersion() noexcept -> std::string;

[[nodiscard]] auto GetGoomLibVersionInfo() noexcept -> std::string;
[[nodiscard]] auto GetGoomLibBuildTime() noexcept -> std::string;

template<typename U, typename T>
[[nodiscard]] constexpr auto ptr_cast(const T ptr) noexcept -> U
{
  static_assert(std::is_pointer_v<T>);
  static_assert(std::is_pointer_v<U>);

  if constexpr (std::is_same_v<U, const std::remove_pointer_t<U>*>)
  {
    return static_cast<U>(static_cast<const void*>(ptr));
  }
  else
  {
    return static_cast<U>(static_cast<void*>(ptr));
  }
}

auto PutFileWithExpandedIncludes(const std::string& includeDir,
                                 const std::string& inFilepath,
                                 const std::string& outFilepath) -> void;

[[nodiscard]] auto GetFileWithExpandedIncludes(const std::string& includeDir,
                                               const std::string& filepath)
    -> std::vector<std::string>;

namespace DETAIL
{

template<std::size_t N>
struct string_literal
{
  consteval string_literal(const char (&arr)[N]) noexcept { std::ranges::copy_n(arr, N, buffer); }

  template<std::size_t LhsSize, std::size_t RhsSize>
    requires(LhsSize + RhsSize - 1 == N)
  consteval string_literal(const string_literal<LhsSize>& lhs,
                           const string_literal<RhsSize>& rhs) noexcept
  {
    auto it = std::ranges::copy(lhs.buffer | std::views::take(LhsSize - 1), buffer).out;
    std::ranges::copy(rhs.buffer, it);
  }

  [[nodiscard]] constexpr auto to_string() const noexcept -> std::string
  {
    return std::string{buffer};
  }
  [[nodiscard]] consteval auto c_str() const noexcept -> const char* { return buffer; }
  [[nodiscard]] constexpr operator std::string() const noexcept { return std::string{buffer}; }
  [[nodiscard]] consteval operator const char*() const noexcept { return buffer; }
  //constexpr operator const char*() const  { return buffer; }

  char buffer[N]{};
  std::size_t size = N;
};

template<std::size_t LhsSize, std::size_t RhsSize>
string_literal(string_literal<LhsSize>, string_literal<RhsSize>)
    -> string_literal<LhsSize + RhsSize - 1>;

template<string_literal String>
consteval auto get_static_buffer() noexcept -> const char (&)[String.size]
{
  return String.buffer;
}

} // namespace DETAIL

template<DETAIL::string_literal cts>
constexpr auto operator""_cts()
{
  return cts;
}

template<std::size_t N>
auto operator+(const std::string& str1, const DETAIL::string_literal<N>& str2)
{
  return str1 + str2.to_string();
}

template<DETAIL::string_literal Lhs, DETAIL::string_literal Rhs>
consteval auto static_concat() noexcept -> DETAIL::string_literal<Lhs.size + Rhs.size - 1>
{
  return DETAIL::string_literal{Lhs, Rhs};
}

template<DETAIL::string_literal Lhs, DETAIL::string_literal Rhs, DETAIL::string_literal... Others>
  requires(sizeof...(Others) != 0)
consteval decltype(auto) static_concat() noexcept
{
  return static_concat<DETAIL::string_literal{Lhs, Rhs}, Others...>();
}

} // namespace GOOM
