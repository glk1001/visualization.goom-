module;

//#undef NO_LOGGING

#include "goom/goom_config.h"
#include "goom/goom_logger.h"

module Goom.Draw.GoomDrawToBuffer;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomTypes;

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
