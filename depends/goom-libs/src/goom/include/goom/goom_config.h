#pragma once

#if defined(REQUIRE_ASSERTS_FOR_ALL_BUILDS) or defined(GOOM_DEBUG)
#undef NDEBUG // NOLINT: Can't include header for this.
//#define DO_GOOM_STATE_DUMP
#endif

#include "goom_utils.h"

#include <cassert>
#include <exception>

namespace GOOM
{
#define Expects(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define Ensures(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define USED_FOR_DEBUGGING(x) ((void)(x)) // NOLINT: Happy with this as a macro.

[[noreturn]] inline auto FailFast() noexcept -> void
{
  std::terminate();
}

inline constexpr auto DATA_DIR               = "data"_cts;
inline constexpr auto MEDIA_DIR              = "media"_cts;
inline constexpr auto FONTS_DIR              = join_paths<DATA_DIR, "fonts">();
inline constexpr auto L_SYSTEMS_DIR          = join_paths<DATA_DIR, "l-systems">();
inline constexpr auto IMAGES_DIR             = join_paths<MEDIA_DIR, "images">();
inline constexpr auto IMAGE_FX_DIR           = join_paths<IMAGES_DIR, "image_fx">();
inline constexpr auto IMAGE_DISPLACEMENT_DIR = join_paths<IMAGES_DIR, "displacements">();

inline constexpr auto RESOURCES_DIR = "resources"_cts;
inline constexpr auto SHADERS_DIR   = join_paths<RESOURCES_DIR, DATA_DIR, "shaders">();

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM
