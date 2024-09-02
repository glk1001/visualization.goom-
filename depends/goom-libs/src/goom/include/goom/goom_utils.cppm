module;

#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

export module Goom.Lib.GoomUtils;

export namespace GOOM
{

auto FindAndReplaceAll(std::string& dataStr,
                       const std::string& searchStr,
                       const std::string& replaceStr) -> void;
auto PutFileLines(std::ostream& outStream, const std::vector<std::string>& lines) -> void;

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

[[nodiscard]] auto GetGoomLibCompilerVersion() noexcept -> std::string;

[[nodiscard]] auto GetGoomLibVersionInfo() noexcept -> std::string;
[[nodiscard]] auto GetGoomLibBuildTime() noexcept -> std::string;

template<typename U, typename T>
[[nodiscard]] constexpr auto ptr_cast(T ptr) noexcept -> U;

auto PutFileWithExpandedIncludes(const std::string& includeDir,
                                 const std::string& inFilepath,
                                 const std::string& outFilepath) -> void;

[[nodiscard]] auto GetFileWithExpandedIncludes(const std::string& includeDir,
                                               const std::string& filepath)
    -> std::vector<std::string>;

} // namespace GOOM

namespace GOOM
{

template<typename U, typename T>
[[nodiscard]] constexpr auto ptr_cast(const T ptr) noexcept -> U
{
  static_assert(std::is_pointer_v<T>);
  static_assert(std::is_pointer_v<U>);

  if constexpr (std::is_same_v<U, const std::remove_pointer_t<U>*>)
  {
    // NOLINTNEXTLINE(bugprone-casting-through-void)
    return static_cast<U>(static_cast<const void*>(ptr));
  }
  else
  {
    // NOLINTNEXTLINE(bugprone-casting-through-void)
    return static_cast<U>(static_cast<void*>(ptr));
  }
}

} // namespace GOOM
