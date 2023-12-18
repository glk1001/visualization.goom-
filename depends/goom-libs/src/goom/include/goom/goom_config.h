#if defined(REQUIRE_ASSERTS_FOR_ALL_BUILDS) or defined(GOOM_DEBUG)
#undef NDEBUG // NOLINT: Can't include header for this.
//#define DO_GOOM_STATE_DUMP
#endif

// *** Put the header guard here to disable CLion's
// *** 'unused include directive' inspection.
#ifndef HDR_GOOM_CONFIG
#define HDR_GOOM_CONFIG

#include "goom_utils.h"

#include <cassert>
#include <exception>
#include <string>

namespace GOOM
{
#define Expects(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define Ensures(cond) assert(cond) // NOLINT: Happy with this as a macro.
#define USED_FOR_DEBUGGING(x) ((void)(x)) // NOLINT: Happy with this as a macro.

[[noreturn]] inline auto FailFast() noexcept -> void
{
  std::terminate();
}

#ifdef _WIN32PC
inline constexpr auto PATH_SEP = "\\"_cts;
#else
inline constexpr auto PATH_SEP = "/"_cts;
#endif

template<DETAIL::string_literal Base>
consteval decltype(auto) join_paths() noexcept
{
  return Base;
}
template<DETAIL::string_literal Base, DETAIL::string_literal... Others>
  requires(sizeof...(Others) != 0)
consteval decltype(auto) join_paths() noexcept
{
  return static_concat<static_concat<Base, PATH_SEP>(), join_paths<Others...>()>();
}
constexpr auto join_paths(const std::string& base) noexcept -> std::string
{
  return base;
}
template<typename... Types>
constexpr auto join_paths(const std::string& base, Types... paths) noexcept -> std::string
{
  return base + PATH_SEP + join_paths(paths...);
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

#endif // HDR_GOOM_CONFIG
