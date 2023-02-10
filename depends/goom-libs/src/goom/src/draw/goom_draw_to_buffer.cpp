//#undef NO_LOGGING

#include "goom_draw_to_buffer.h"

#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::DRAW
{

GoomDrawToSingleBuffer::GoomDrawToSingleBuffer(const Dimensions& dimensions,
                                               GoomLogger& goomLogger) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{goomLogger}
{
  USED_FOR_DEBUGGING(m_goomLogger);
}

GoomDrawToTwoBuffers::GoomDrawToTwoBuffers(const Dimensions& dimensions,
                                           GoomLogger& goomLogger) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{goomLogger}
{
  USED_FOR_DEBUGGING(m_goomLogger);
}

} // namespace GOOM::DRAW
