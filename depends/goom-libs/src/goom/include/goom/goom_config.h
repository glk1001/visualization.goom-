#pragma once

#if defined(REQUIRE_ASSERTS_FOR_ALL_BUILDS) or defined(GOOM_DEBUG)
#undef NDEBUG // NOLINT: Can't include header for this.
//#define DO_GOOM_STATE_DUMP
#endif

#ifndef __APPLE__
#include "goom_utils.h"
#endif

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

#ifdef __APPLE__
// TODO(glk) - Remove this when Apple clang gets it's act together.
#define DATA_DIR "data"
#define MEDIA_DIR "media"
#define FONTS_DIR \
  DATA_DIR "/" \
           "fonts"
#define L_SYSTEMS_DIR \
  DATA_DIR "/" \
           "l-systems"
#define IMAGES_DIR \
  MEDIA_DIR "/" \
            "images"
#define IMAGE_FX_DIR \
  IMAGES_DIR "/" \
             "image_fx"
#define IMAGE_DISPLACEMENT_DIR \
  IMAGES_DIR "/" \
             "displacements"

#define RESOURCES_DIR "resources"
#define SHADERS_DIR \
  RESOURCES_DIR "/" DATA_DIR "/" \
                "shaders"
#else
inline constexpr auto DATA_DIR               = "data"_cts;
inline constexpr auto MEDIA_DIR              = "media"_cts;
inline constexpr auto FONTS_DIR              = join_paths<DATA_DIR, "fonts">();
inline constexpr auto L_SYSTEMS_DIR          = join_paths<DATA_DIR, "l-systems">();
inline constexpr auto IMAGES_DIR             = join_paths<MEDIA_DIR, "images">();
inline constexpr auto IMAGE_FX_DIR           = join_paths<IMAGES_DIR, "image_fx">();
inline constexpr auto IMAGE_DISPLACEMENT_DIR = join_paths<IMAGES_DIR, "displacements">();

inline constexpr auto RESOURCES_DIR = "resources"_cts;
inline constexpr auto SHADERS_DIR   = join_paths<RESOURCES_DIR, DATA_DIR, "shaders">();
#endif

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM
