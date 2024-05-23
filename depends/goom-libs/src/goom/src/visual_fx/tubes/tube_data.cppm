module;

#include "goom/point2d.h"

#include <cstdint>
#include <functional>

namespace GOOM
{
class GoomTime;
}

module Goom.VisualFx.TubesFx:TubeData;

import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::VISUAL_FX::TUBES
{

using DrawLineFunc   = std::function<void(
    Point2dInt point1, Point2dInt point2, const DRAW::MultiplePixels& colors, uint8_t thickness)>;
using DrawCircleFunc = std::function<void(
    Point2dInt point, int radius, const DRAW::MultiplePixels& colors, uint8_t thickness)>;
using DrawSmallImageFunc =
    std::function<void(Point2dInt midPoint,
                       UTILS::GRAPHICS::SmallImageBitmaps::ImageNames imageName,
                       uint32_t size,
                       const DRAW::MultiplePixels& colors)>;

struct TubeDrawFuncs
{
  DrawLineFunc drawLine;
  DrawCircleFunc drawCircle;
  DrawSmallImageFunc drawSmallImage;
};

struct TubeData
{
  uint32_t tubeId{};
  TubeDrawFuncs drawFuncs{};
  uint32_t screenWidth{};
  uint32_t screenHeight{};
  const GoomTime* goomTime{};
  const UTILS::MATH::IGoomRand* goomRand{};
  COLOR::WeightedRandomColorMaps mainColorMaps{};
  COLOR::WeightedRandomColorMaps lowColorMaps{};
  float radiusEdgeOffset{};
  float brightnessFactor{};
};

} // namespace GOOM::VISUAL_FX::TUBES
