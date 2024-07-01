module;

#include <unordered_set>

module Goom.Control.GoomRandomStateHandler;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using UTILS::NUM;
using UTILS::MATH::Weights;

GoomRandomStateHandler::GoomRandomStateHandler(const IGoomRand& goomRand) : m_goomRand{&goomRand}
{
  ChangeToNextState();
}

auto GoomRandomStateHandler::ChangeToNextState() -> void
{
}

auto GoomRandomStateHandler::GetFullDrawablesPool() -> std::unordered_set<GoomDrawables>
{
  auto fullPool = std::unordered_set<GoomDrawables>{};

  for (auto i = 0U; i < NUM<GoomDrawables>; ++i)
  {
    fullPool.insert(static_cast<GoomDrawables>(i));
  }

  return fullPool;
}

} // namespace GOOM::CONTROL
