#pragma once

#include "colormap_enums.h"

#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
namespace COLOR_DATA
{
#else
namespace GOOM::COLOR::COLOR_DATA
{
#endif

extern const std::vector<ColorMapName> WES_ANDERSON_MAPS;
extern const std::vector<ColorMapName> BLUE_MAPS;
extern const std::vector<ColorMapName> RED_MAPS;
extern const std::vector<ColorMapName> GREEN_MAPS;
extern const std::vector<ColorMapName> YELLOW_MAPS;
extern const std::vector<ColorMapName> ORANGE_MAPS;
extern const std::vector<ColorMapName> PURPLE_MAPS;
extern const std::vector<ColorMapName> CITY_MAPS;
extern const std::vector<ColorMapName> SEASON_MAPS;
extern const std::vector<ColorMapName> HEAT_MAPS;
extern const std::vector<ColorMapName> COLD_MAPS;
extern const std::vector<ColorMapName> PASTEL_MAPS;

#if __cplusplus <= 201402L
} // namespace COLOR_DATA
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR::COLOR_DATA
#endif
