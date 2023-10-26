#pragma once

#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "utils/buffer_saver.h"

#include <format> // NOLINT: Waiting to use C++20.
#include <string>

namespace GOOM
{

template<class HeaderT>
using GoomPixelBufferSaver = class UTILS::BufferSaver<Pixel, HeaderT>;

template<class HeaderT>
using GoomPoint2dIntBufferSaver = UTILS::BufferSaver<Point2dInt, HeaderT>;
template<class HeaderT>
using GoomPoint2dFltBufferSaver = UTILS::BufferSaver<Point2dFlt, HeaderT>;

[[nodiscard]] auto GetBufferValueToString(const Pixel& bufferValue) -> std::string;

[[nodiscard]] auto GetBufferValueToString(const Point2dInt& bufferValue) -> std::string;
[[nodiscard]] auto GetBufferValueToString(const Point2dFlt& bufferValue) -> std::string;


inline auto GetBufferValueToString(const Pixel& bufferValue) -> std::string
{
  // NOLINTNEXTLINE: Waiting to use C++20.
  return std_fmt::format("{:6d} {:6d} {:6d} {:6d}",
                         bufferValue.R(),
                         bufferValue.G(),
                         bufferValue.B(),
                         bufferValue.A());
}

inline auto GetBufferValueToString(const Point2dInt& bufferValue) -> std::string
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_fmt::format("{:6d} {:6d}", bufferValue.x, bufferValue.y);
}

inline auto GetBufferValueToString(const Point2dFlt& bufferValue) -> std::string
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_fmt::format("{:6.2f} {:6.2f}", bufferValue.x, bufferValue.y);
}

} // namespace GOOM
