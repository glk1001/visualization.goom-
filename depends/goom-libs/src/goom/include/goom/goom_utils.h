#pragma once

#include <algorithm>
#include <array>
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

// NOLINTBEGIN: Tricky template stuff here.
namespace DETAIL
{

template<std::size_t N>
struct compile_time_string
{
  consteval compile_time_string(const char (&arr)[N]) noexcept
  {
    std::ranges::copy_n(arr, N, buffer);
  }

  template<std::size_t LhsSize, std::size_t RhsSize>
    requires(((LhsSize + RhsSize) - 1) == N)
  consteval compile_time_string(const compile_time_string<LhsSize>& lhs,
                                const compile_time_string<RhsSize>& rhs) noexcept
  {
    auto it = std::ranges::copy(lhs.buffer | std::views::take(LhsSize - 1), buffer).out;
    std::ranges::copy(rhs.buffer, it);
  }

  [[nodiscard]] constexpr auto to_string() const noexcept -> std::string
  {
    return std::string{buffer};
  }
  [[nodiscard]] constexpr auto c_str() const noexcept -> const char* { return buffer; }
  [[nodiscard]] constexpr operator std::string() const noexcept { return std::string{buffer}; }
  [[nodiscard]] consteval operator const char*() const noexcept { return buffer; }
  [[nodiscard]] consteval auto to_array() const noexcept -> std::array<char, N>
  {
    return std::to_array(buffer);
  }

  char buffer[N]{};
  std::size_t size = N;
};

template<std::size_t LhsSize, std::size_t RhsSize>
compile_time_string(compile_time_string<LhsSize>, compile_time_string<RhsSize>)
    -> compile_time_string<(LhsSize + RhsSize) - 1>;

} // namespace DETAIL
// NOLINTEND: Tricky template stuff here.

template<DETAIL::compile_time_string Cts>
consteval auto operator""_cts()
{
  return Cts;
}

template<DETAIL::compile_time_string Lhs, DETAIL::compile_time_string Rhs>
consteval auto static_concat() noexcept -> DETAIL::compile_time_string<(Lhs.size + Rhs.size) - 1>
{
  return DETAIL::compile_time_string{Lhs, Rhs};
}

template<DETAIL::compile_time_string Lhs,
         DETAIL::compile_time_string Rhs,
         DETAIL::compile_time_string... Others>
  requires(sizeof...(Others) != 0)
consteval auto static_concat() noexcept -> decltype(auto)
{
  return static_concat<DETAIL::compile_time_string{Lhs, Rhs}, Others...>();
}

template<std::size_t N>
auto operator+(const std::string& str1, const DETAIL::compile_time_string<N>& str2)
{
  return str1 + str2.to_string();
}

namespace DETAIL
{

[[nodiscard]] consteval auto GetPathSep() noexcept -> decltype(auto)
{
#ifdef _WIN32PC
  return "\\"_cts;
#else
  return "/"_cts;
#endif
}

} // namespace DETAIL

template<DETAIL::compile_time_string Base>
consteval auto join_paths() noexcept -> decltype(auto)
{
  return Base;
}
template<DETAIL::compile_time_string Base, DETAIL::compile_time_string... Others>
  requires(sizeof...(Others) != 0)
consteval auto join_paths() noexcept -> decltype(auto)
{
  return static_concat<static_concat<Base, DETAIL::GetPathSep()>(), join_paths<Others...>()>();
}

constexpr auto join_paths(const std::string& base) noexcept -> std::string
{
  return base;
}
template<typename... Types>
  requires(sizeof...(Types) != 0)
constexpr auto join_paths(const std::string& base, Types... paths) noexcept -> std::string
{
  return base + DETAIL::GetPathSep() + join_paths(paths...);
}

} // namespace GOOM
