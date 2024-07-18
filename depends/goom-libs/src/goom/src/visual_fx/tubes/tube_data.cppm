module;

#include <cstdint>
#include <functional>

module Goom.VisualFx.TubesFx:TubeData;

import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.GoomTime;
import Goom.Lib.Point2d;

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
  uint32_t tubeId = 0U;
  TubeDrawFuncs drawFuncs{};
  uint32_t screenWidth                   = 0U;
  uint32_t screenHeight                  = 0U;
  const UTILS::GoomTime* goomTime        = nullptr;
  const UTILS::MATH::GoomRand* goomRand = nullptr;
  COLOR::WeightedRandomColorMaps mainColorMaps;
  COLOR::WeightedRandomColorMaps lowColorMaps;
  float radiusEdgeOffset = 0.0F;
  float brightnessFactor = 0.0F;
};

} // namespace GOOM::VISUAL_FX::TUBES
