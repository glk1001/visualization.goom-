module;

#include <cstdint>

module Goom.VisualFx.CirclesFx:Helper;

import :BitmapGetter;

namespace GOOM::VISUAL_FX::CIRCLES
{

struct Helper
{
  uint32_t lineDotDiameter;
  uint32_t minDotDiameter;
  uint32_t maxDotDiameter;
  const IBitmapGetter* bitmapGetter;
};

} // namespace GOOM::VISUAL_FX::CIRCLES
