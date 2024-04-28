module;

#include "goom/goom_config.h"
#include "goom/goom_graphic.h"

#include <format>
#include <string>

export module Goom.Utils:FormatUtils;

export namespace GOOM::UTILS::FMT
{

template<typename TPoint>
[[nodiscard]] inline auto Pt(const char* const fmtStr, const TPoint& point) -> std::string
{
  const auto format = std::format("({}, {})", fmtStr, fmtStr);
  return std::vformat(format, std::make_format_args(point.x, point.y));
}

[[nodiscard]] inline auto Rgba(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string(fmtStr) == "{}" ? "{:5d}" : fmtStr;
  const auto format = std::format("{}, {}, {}, {}", bestFmtStr, bestFmtStr, bestFmtStr, bestFmtStr);
  const auto red    = pixel.R();
  const auto green  = pixel.G();
  const auto blue   = pixel.B();
  const auto alpha  = pixel.A();
  return std::vformat(format, std::make_format_args(red, green, blue, alpha));
}

[[nodiscard]] inline auto RgbF(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string(fmtStr) == "{}" ? "{:.2f}" : fmtStr;
  const auto format            = std::format("({}, {}, {})", bestFmtStr, bestFmtStr, bestFmtStr);
  const auto red               = pixel.RFlt();
  const auto green             = pixel.GFlt();
  const auto blue              = pixel.BFlt();
  return std::vformat(format, std::make_format_args(red, green, blue));
}

} // namespace GOOM::UTILS::FMT

// NOLINTNEXTLINE(misc-unused-alias-decls): It is actually used!?
namespace FMT = GOOM::UTILS::FMT;
