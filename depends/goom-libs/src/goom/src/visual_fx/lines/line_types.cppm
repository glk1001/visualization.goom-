module;

module Goom.VisualFx.LinesFx:LineTypes;

import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

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
