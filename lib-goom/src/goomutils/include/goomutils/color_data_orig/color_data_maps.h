#pragma once

// IMPORTANT - THIS CODE IS GENERATED BY xxxx - DO NOT EDIT

#include "color_data/colormap_enums.h"
#include "vivid/types.h"

#include <array>
#include <vector>

#if __cplusplus > 201402L
namespace GOOM::UTILS::COLOR_DATA
{
#else
namespace GOOM
{
namespace UTILS
{
namespace COLOR_DATA
{
#endif

// array of raw maps matching elements of enum 'ColorMapName'
struct ColorNamePair
{
  ColorMapName colorMapName;
  const std::vector<vivid::srgb_t>* vividArray;
};

extern const std::array<ColorNamePair, 1687> ALL_MAPS;

extern const std::vector<ColorMapName> PERC_UNIF_SEQUENTIAL_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL2_MAPS;
extern const std::vector<ColorMapName> DIVERGING_MAPS;
extern const std::vector<ColorMapName> DIVERGING_BLACK_MAPS;
extern const std::vector<ColorMapName> QUALITATIVE_MAPS;
extern const std::vector<ColorMapName> MISC_MAPS;
extern const std::vector<ColorMapName> CYCLIC_MAPS;
extern const std::vector<ColorMapName> PERC_UNIF_SEQUENTIAL_SLIM_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL_SLIM_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL2_SLIM_MAPS;
extern const std::vector<ColorMapName> DIVERGING_SLIM_MAPS;
extern const std::vector<ColorMapName> DIVERGING_BLACK_SLIM_MAPS;
extern const std::vector<ColorMapName> QUALITATIVE_SLIM_MAPS;
extern const std::vector<ColorMapName> MISC_SLIM_MAPS;
extern const std::vector<ColorMapName> CYCLIC_SLIM_MAPS;

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
