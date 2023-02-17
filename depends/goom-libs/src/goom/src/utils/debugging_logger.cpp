#undef NO_LOGGING

#include "debugging_logger.h"

#include "goom_config.h"
#include "goom_logger.h"

namespace GOOM::UTILS
{

namespace
{
GoomLogger* globalGoomLogger{}; // NOLINT: For debugging use only.
}

auto SetGoomLogger(GoomLogger& goomLogger) noexcept -> void
{
  globalGoomLogger = &goomLogger;
}

auto GetGoomLogger() noexcept -> GoomLogger&
{
  Expects(globalGoomLogger != nullptr);
  return *globalGoomLogger;
}

} // namespace GOOM::UTILS
