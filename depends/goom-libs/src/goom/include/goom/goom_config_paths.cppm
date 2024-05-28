export module Goom.Lib.GoomConfigPaths;

import Goom.Lib.GoomPaths;

export namespace GOOM
{

inline constexpr auto DATA_DIR               = "data"_cts;
inline constexpr auto MEDIA_DIR              = "media"_cts;
inline constexpr auto FONTS_DIR              = join_paths<DATA_DIR, "fonts">();
inline constexpr auto L_SYSTEMS_DIR          = join_paths<DATA_DIR, "l-systems">();
inline constexpr auto IMAGES_DIR             = join_paths<MEDIA_DIR, "images">();
inline constexpr auto IMAGE_FX_DIR           = join_paths<IMAGES_DIR, "image_fx">();
inline constexpr auto IMAGE_DISPLACEMENT_DIR = join_paths<IMAGES_DIR, "displacements">();

inline constexpr auto RESOURCES_DIR = "resources"_cts;
inline constexpr auto SHADERS_DIR   = join_paths<RESOURCES_DIR, DATA_DIR, "shaders">();

} // namespace GOOM
