module;

//#undef NO_LOGGING

#include "goom/goom_config.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"

module Goom.Draw.GoomDrawToBuffer;

import Goom.Draw.GoomDrawBase;

namespace GOOM::DRAW
{

GoomDrawToSingleBuffer::GoomDrawToSingleBuffer(const Dimensions& dimensions,
                                               GoomLogger& goomLogger) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{&goomLogger}
{
}

GoomDrawToTwoBuffers::GoomDrawToTwoBuffers(const Dimensions& dimensions,
                                           GoomLogger& goomLogger) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{&goomLogger}
{
}

} // namespace GOOM::DRAW
