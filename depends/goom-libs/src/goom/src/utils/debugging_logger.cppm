module;

#undef NO_LOGGING

#define REQUIRE_ASSERTS_FOR_ALL_BUILDS // Check for non-null pointers.

#include "goom/goom_config.h"
#include "goom/goom_logger.h"

export module Goom.Utils.DebuggingLogger;

export namespace GOOM::UTILS
{

auto SetGoomLogger(GoomLogger& goomLogger) noexcept -> void;
[[nodiscard]] auto GetGoomLogger() noexcept -> GoomLogger&;

} // namespace GOOM::UTILS

module :private;

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
