//#undef NO_LOGGING

#include "goom_draw_to_buffer.h"

#include "goom_config.h"
#include "goom_draw.h"
#include "goom_logger.h"
#include "goom_types.h"

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
