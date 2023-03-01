#ifdef GOOM_DEBUG
#undef NDEBUG
//#define DO_GOOM_STATE_DUMP
#endif

// *** Put the header guard here to disable CLion's
// *** 'unused include directive' inspection.
#ifndef HDR_GOOM_CONFIG
#define HDR_GOOM_CONFIG

#include <exception>
// NOLINTBEGIN: Need special macros with these names.
#define USED_FOR_DEBUGGING(x) ((void)(x))

#include <cassert>
#define Expects(cond) assert(cond)
#define Ensures(cond) assert(cond)
[[noreturn]] inline auto FailFast() noexcept -> void
{
  std::terminate();
}
// NOLINTEND

namespace GOOM
{

#ifdef _WIN32PC
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

// TODO(glk) - With C++20 we should be able to do 'inline constexpr FONTS_DIR = ...', etc.
#define DATA_DIR "data"
#define MEDIA_DIR "media"
#define FONTS_DIR DATA_DIR PATH_SEP "fonts"
#define L_SYSTEMS_DIR DATA_DIR PATH_SEP "l-systems"
#define IMAGES_DIR MEDIA_DIR PATH_SEP "images"
#define IMAGE_FX_DIR IMAGES_DIR PATH_SEP "image_fx"
#define IMAGE_DISPLACEMENT_DIR IMAGES_DIR PATH_SEP "displacements"

#define RESOURCES_DIR "resources"
#define SHADERS_DIR RESOURCES_DIR PATH_SEP DATA_DIR PATH_SEP "shaders"

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM

#endif // HDR_GOOM_CONFIG
