#pragma once

namespace GOOM
{

#ifdef _WIN32PC
constexpr const char* PATH_SEP = "\\";
#else
constexpr auto PATH_SEP = "/";
#endif

constexpr auto FONTS_DIR = "fonts";
constexpr auto IMAGES_DIR = "images";
constexpr auto IMAGE_DISPLACEMENT_DIR = "displacements";

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

} // namespace GOOM