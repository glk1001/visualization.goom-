module;

//#undef NO_LOGGING

#include "goom/goom_logger.h"

module Goom.Draw.GoomDrawToBuffer;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

namespace GOOM::DRAW
{

GoomDrawToSingleBuffer::GoomDrawToSingleBuffer(const Dimensions& dimensions,
                                               GoomLogger& goomLogger,
                                               PixelBuffer& buffer) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{&goomLogger}, m_buffer{&buffer}
{
  Expects(buffer.GetWidth() == dimensions.GetWidth());
}

GoomDrawToTwoBuffers::GoomDrawToTwoBuffers(const Dimensions& dimensions,
                                           GoomLogger& goomLogger,
                                           PixelBuffer& buffer1,
                                           PixelBuffer& buffer2) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{&goomLogger}, m_buffer1{&buffer1}, m_buffer2{&buffer2}
{
  Expects(buffer1.GetWidth() == dimensions.GetWidth());
  Expects(buffer2.GetWidth() == dimensions.GetWidth());
}

} // namespace GOOM::DRAW
