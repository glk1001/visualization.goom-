module;

#include <cstdint>

export module Goom.VisualFx.CirclesFx.Helper;

import Goom.VisualFx.CirclesFx.BitmapGetter;

export namespace GOOM::VISUAL_FX::CIRCLES
{

struct Helper
{
  uint32_t lineDotDiameter;
  uint32_t minDotDiameter;
  uint32_t maxDotDiameter;
  const IBitmapGetter* bitmapGetter;
};

} // namespace GOOM::VISUAL_FX::CIRCLES
