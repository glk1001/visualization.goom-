#pragma once

#include "bitmap_getter_base.h"

#include <cstdint>

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
