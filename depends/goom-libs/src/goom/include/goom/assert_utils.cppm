module;

#include <exception>
#ifdef _MSC_VER // TODO(glk): Bugged in MSVC??
#include <iostream>
#else
#include <format>
#include <print>
#endif
#include <source_location>

export module Goom.Lib.AssertUtils;

#ifdef GOOM_DEBUG
static constexpr auto ENABLE_ASSERTS = true;
#ifndef _MSC_VER // TODO(glk): Bugged in MSVC??
#ifdef NDEBUG
#error "Should not be set for GOOM_DEBUG"
#endif
#endif
#else
static constexpr auto ENABLE_ASSERTS = false;
#endif

export namespace GOOM
{

inline constexpr auto ASSERTS_ENABLED = ENABLE_ASSERTS;

constexpr auto Expects(
    bool expr,
    const std::string& extraInfo         = "",
    const std::source_location& location = std::source_location::current()) noexcept -> void;

constexpr auto Ensures(
    bool expr,
    const std::string& extraInfo         = "",
    const std::source_location& location = std::source_location::current()) noexcept -> void;

constexpr auto Assert(
    bool expr,
    const std::string& extraInfo         = "",
    const std::source_location& location = std::source_location::current()) noexcept -> void;

[[noreturn]] auto FailFast() noexcept -> void;

} // namespace GOOM

namespace GOOM
{

constexpr auto Check(const bool expr,
                     // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                     const std::string& reason,
                     const std::string& extraInfo,
                     const std::source_location& location) noexcept -> void
{
  if (expr)
  {
    return;
  }

  try
  {
#ifdef _MSC_VER
    // TODO(glk): Bugged in MSVC??
    const auto extraInfoStr = extraInfo;
    std::cerr << reason << extraInfoStr << " - " << location.file_name() << ":" << location.line()
              << "\n";
#else
    const auto extraInfoStr = extraInfo.empty() ? std::string{} : std::format(" ({})", extraInfo);
    std::println("{}{} - {}:{}", reason, extraInfoStr, location.file_name(), location.line());
#endif

    std::terminate();
  }
  catch (...)
  {
    std::terminate();
  }
}

constexpr auto Expects(const bool expr,
                       const std::string& extraInfo,
                       const std::source_location& location) noexcept -> void
{
  if constexpr (ASSERTS_ENABLED)
  {
    Check(expr, "Expects FAILED", extraInfo, location);
  }
}

constexpr auto Ensures(const bool expr,
                       const std::string& extraInfo,
                       const std::source_location& location) noexcept -> void
{
  if constexpr (ASSERTS_ENABLED)
  {
    Check(expr, "Ensures FAILED", extraInfo, location);
  }
}

constexpr auto Assert(const bool expr,
                      const std::string& extraInfo,
                      const std::source_location& location) noexcept -> void
{
  if constexpr (ASSERTS_ENABLED)
  {
    Check(expr, "Assert FAILED", extraInfo, location);
  }
}

[[noreturn]] auto FailFast() noexcept -> void
{
  std::terminate();
}

} // namespace GOOM
