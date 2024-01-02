#pragma once

#include "goom/goom_config.h"
#include "goom/goom_graphic.h"

#include <format>
#include <string>

namespace GOOM::UTILS::FMT
{

#ifdef __APPLE__
template<typename TPoint>
[[nodiscard]] [[noreturn]] inline auto Pt([[maybe_unused]] const char* const fmtStr,
                                          [[maybe_unused]] const TPoint& point) -> std::string
{
  // TODO(glk) - Remove this when Apple clang gets it's act together.
  FailFast();
}
#else
template<typename TPoint>
[[nodiscard]] inline auto Pt(const char* const fmtStr, const TPoint& point) -> std::string
{
  const auto format = std::format("({}, {})", fmtStr, fmtStr);
  return std::vformat(format, std::make_format_args(point.x, point.y));
}
#endif

#ifdef __APPLE__
[[nodiscard]] [[noreturn]] inline auto Rgba([[maybe_unused]] const char* const fmtStr,
                                            [[maybe_unused]] const Pixel& pixel) -> std::string
{
  // TODO(glk) - Remove this when Apple clang gets it's act together.
  FailFast();
}
#else
[[nodiscard]] inline auto Rgba(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string(fmtStr) == "{}" ? "{:5d}" : fmtStr;
  const auto format = std::format("{}, {}, {}, {}", bestFmtStr, bestFmtStr, bestFmtStr, bestFmtStr);
  return std::vformat(format, std::make_format_args(pixel.R(), pixel.G(), pixel.B(), pixel.A()));
}
#endif

#ifdef __APPLE__
[[nodiscard]] [[noreturn]] inline auto RgbF([[maybe_unused]] const char* const fmtStr,
                                            [[maybe_unused]] const Pixel& pixel) -> std::string
{
  // TODO(glk) - Remove this when Apple clang gets it's act together.
  FailFast();
}
#else
[[nodiscard]] inline auto RgbF(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string(fmtStr) == "{}" ? "{:.2f}" : fmtStr;
  const auto format            = std::format("({}, {}, {})", bestFmtStr, bestFmtStr, bestFmtStr);
  return std::vformat(format, std::make_format_args(pixel.RFlt(), pixel.GFlt(), pixel.BFlt()));
}
#endif

} // namespace GOOM::UTILS::FMT

// NOLINTNEXTLINE(misc-unused-alias-decls): It is actually used!?
namespace FMT = GOOM::UTILS::FMT;
