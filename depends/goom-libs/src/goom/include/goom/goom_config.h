#pragma once

#ifdef GOOM_DEBUG
#undef NDEBUG
#endif

#define UNUSED_FOR_NDEBUG(x) ((void)(x))

#include <cassert>
#define Expects(cond) assert(cond)
#define Ensures(cond) assert(cond)
#define FailFast() assert(false) // NOLINT

namespace GOOM
{

#ifdef _WIN32PC
static constexpr const char* PATH_SEP = "\\";
#else
static constexpr auto* PATH_SEP = "/";
#endif

static constexpr auto* FONTS_DIR = "fonts";
static constexpr auto* IMAGES_DIR = "images";
static constexpr auto* IMAGE_DISPLACEMENT_DIR = "displacements";

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM
