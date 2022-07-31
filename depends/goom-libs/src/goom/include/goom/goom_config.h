#pragma once

#ifdef GOOM_DEBUG
#undef NDEBUG
//#define DO_GOOM_STATE_DUMP
#endif

// NOLINTBEGIN: Need special macros with these names.
#define UNUSED_FOR_NDEBUG(x) ((void)(x))

#include <cassert>
#define Expects(cond) assert(cond)
#define Ensures(cond) assert(cond)
#define FailFast() assert(false)
// NOLINTEND

namespace GOOM
{

#ifdef _WIN32PC
static constexpr auto PATH_SEP = "\\";
#else
static constexpr auto PATH_SEP = "/";
#endif

static constexpr auto FONTS_DIR              = "fonts";
static constexpr auto IMAGES_DIR             = "images";
static constexpr auto IMAGE_DISPLACEMENT_DIR = "displacements";

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM
