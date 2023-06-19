#pragma once

#include "goom_graphic.h"
#include "point2d.h"

namespace GOOM
{

struct FilterPosArrays
{
  std_spn::span<Point2dFlt> filterSrcePos{};
  bool filterSrcePosNeedsUpdating{};
  std_spn::span<Point2dFlt> filterDestPos{};
  bool filterDestPosNeedsUpdating{};
};
struct ImageArrays
{
  GOOM::PixelBuffer mainImageData{};
  bool mainImageDataNeedsUpdating{};
  GOOM::PixelBuffer lowImageData{};
  bool lowImageDataNeedsUpdating{};
};
struct MiscData
{
  // TODO - blending params, contrast, other command/effects
  float lerpFactor = 0.0F;
  float brightness = 1.0F;
};
struct FrameData
{
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM
