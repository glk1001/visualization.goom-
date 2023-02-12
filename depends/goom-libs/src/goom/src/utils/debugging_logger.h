#include "goom_logger.h"

// *** Put the header guard here to disable CLion's
// *** 'unused include directive' inspection.
#ifndef HDR_DEBUGGING_LOGGER
#define HDR_DEBUGGING_LOGGER

namespace GOOM::UTILS
{

auto SetGoomLogger(GoomLogger& goomLogger) noexcept -> void;
[[nodiscard]] auto GetGoomLogger() noexcept -> GoomLogger&;

} // namespace GOOM::UTILS

#endif // HDR_DEBUGGING_LOGGER
