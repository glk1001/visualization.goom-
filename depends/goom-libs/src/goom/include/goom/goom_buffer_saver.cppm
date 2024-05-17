module;

#include "goom/goom_config.h"

#include <format>
#include <string>

export module Goom.Lib.GoomBufferSaver;

import Goom.Utils.BufferSaver;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

export namespace GOOM
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

} // namespace GOOM

namespace GOOM
{

inline auto GetBufferValueToString(const Pixel& bufferValue) -> std::string
{
  return std::format("{:6d} {:6d} {:6d} {:6d}",
                     bufferValue.R(),
                     bufferValue.G(),
                     bufferValue.B(),
                     bufferValue.A());
}

inline auto GetBufferValueToString(const Point2dInt& bufferValue) -> std::string
{
  return std::format("{:6d} {:6d}", bufferValue.x, bufferValue.y);
}

inline auto GetBufferValueToString(const Point2dFlt& bufferValue) -> std::string
{
  return std::format("{:6.2f} {:6.2f}", bufferValue.x, bufferValue.y);
}

} // namespace GOOM
