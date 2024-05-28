#pragma once

#if defined(REQUIRE_ASSERTS_FOR_ALL_BUILDS) or defined(GOOM_DEBUG)
#undef NDEBUG // NOLINT: Can't include header for this.
//#define DO_GOOM_STATE_DUMP
#endif

#include <cassert>
#include <exception>

namespace GOOM
{
inline constexpr auto MIN_NORMALIZED_COORD   = -2.0F;
inline constexpr auto MAX_NORMALIZED_COORD   = -MIN_NORMALIZED_COORD;
inline constexpr auto NORMALIZED_COORD_WIDTH = MAX_NORMALIZED_COORD - MIN_NORMALIZED_COORD;

#define Expects(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define Ensures(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define USED_FOR_DEBUGGING(x) ((void)(x)) // NOLINT: Happy with this as a macro.

[[noreturn]] inline auto FailFast() noexcept -> void
{
  std::terminate();
}

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM
