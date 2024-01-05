#pragma once

#include "goom/goom_graphic.h"
#include "goom/goom_types.h"

namespace GOOM::VISUAL_FX::LINES
{

enum class LineType : UnderlyingEnumType
{
  CIRCLE = 0, // (param = radius)
  H_LINE, // (param = y)
  V_LINE, // (param = x)
};

struct LineParams
{
  LineType lineType{};
  float param{};
  Pixel color;
  float amplitude{};
};


} // namespace GOOM::VISUAL_FX::LINES
