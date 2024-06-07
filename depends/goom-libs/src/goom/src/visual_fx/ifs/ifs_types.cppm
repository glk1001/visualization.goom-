module;

#include <cstdint>

module Goom.VisualFx.IfsDancersFx:IfsTypes;

import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::VISUAL_FX::IFS
{

using Dbl = float;
using Flt = int32_t;

struct FltPoint
{
  Flt x = 0;
  Flt y = 0;
};

inline constexpr auto FIX = 12U;

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

[[nodiscard]] inline auto MultByUnit(const Dbl x) -> Flt
{
  static constexpr auto UNIT = 1U << FIX;
  return static_cast<Flt>(static_cast<Dbl>(UNIT) * x);
}

[[nodiscard]] inline auto DivByUnit(const Flt x) -> Flt
{
  return x >> FIX; // NOLINT(hicpp-signed-bitwise)
}

[[nodiscard]] inline auto DivBy2Units(const Flt x) -> Flt
{
  return x >> (FIX + 1); // NOLINT(hicpp-signed-bitwise)
}

} // namespace GOOM::VISUAL_FX::IFS
