#undef NO_LOGGING

#include "debugging_logger.h"

#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::UTILS
{

namespace
{
GoomLogger* s_goomLogger{};
}

auto SetGoomLogger(GoomLogger& goomLogger) noexcept -> void
{
  s_goomLogger = &goomLogger;
}

auto GetGoomLogger() noexcept -> GoomLogger&
{
  Expects(s_goomLogger != nullptr);
  return *s_goomLogger;
}

} // namespace GOOM::UTILS
