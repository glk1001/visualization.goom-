#pragma once

#include <cstdint>
#include <span>
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

template<typename T, typename U>
[[nodiscard]] constexpr auto GetConstSpan(const U& container) noexcept -> std_spn::span<const T>
{
  return std_spn::span<const T>{container.data(), container.size()};
}

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

} // namespace GOOM
