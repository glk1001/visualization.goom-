#pragma once

namespace GOOM
{
class GoomLogger;
class PluginInfo;

namespace DRAW
{
class IGoomDraw;
}
namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX
{

struct FxHelper
{
  DRAW::IGoomDraw* draw;
  const PluginInfo* goomInfo;
  const UTILS::MATH::IGoomRand* goomRand;
  GoomLogger* goomLogger;
};

} // namespace VISUAL_FX
} // namespace GOOM
