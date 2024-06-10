module;

#include <format>
#include <string>

export module Goom.Utils.FormatUtils;

import Goom.Lib.GoomGraphic;

export namespace GOOM::UTILS::FMT
{

template<typename TPoint>
[[nodiscard]] auto Pt(const char* fmtStr, const TPoint& point) -> std::string;

[[nodiscard]] auto Rgba(const char* fmtStr, const Pixel& pixel) -> std::string;
[[nodiscard]] auto RgbF(const char* fmtStr, const Pixel& pixel) -> std::string;

} // namespace GOOM::UTILS::FMT

namespace GOOM::UTILS::FMT
{

template<typename TPoint>
[[nodiscard]] auto Pt(const char* const fmtStr, const TPoint& point) -> std::string
{
  const auto format = std::vformat("({}, {})", std::make_format_args(fmtStr, fmtStr));
  return std::vformat(format, std::make_format_args(point.x, point.y));
}

[[nodiscard]] auto Rgba(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string{fmtStr} == "{}" ? "{:5d}" : fmtStr;
  const auto format            = std::vformat(
      "({}, {}, {}, {})", std::make_format_args(bestFmtStr, bestFmtStr, bestFmtStr, bestFmtStr));
  const auto red   = pixel.R();
  const auto green = pixel.G();
  const auto blue  = pixel.B();
  const auto alpha = pixel.A();
  return std::vformat(format, std::make_format_args(red, green, blue, alpha));
}

[[nodiscard]] auto RgbF(const char* const fmtStr, const Pixel& pixel) -> std::string
{
  const auto* const bestFmtStr = std::string{fmtStr} == "{}" ? "{:.2f}" : fmtStr;
  const auto format =
      std::vformat("({}, {}, {})", std::make_format_args(bestFmtStr, bestFmtStr, bestFmtStr));
  const auto red   = pixel.RFlt();
  const auto green = pixel.GFlt();
  const auto blue  = pixel.BFlt();
  return std::vformat(format, std::make_format_args(red, green, blue));
}

} // namespace GOOM::UTILS::FMT
