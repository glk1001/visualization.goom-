#include "color/random_colormaps.h"
#include "goom_all_visual_fx.h"
#include "utils/enumutils.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tube_fx.h"

#include <functional>
#include <map>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using COLOR::GetAllMapsUnweighted;
using COLOR::GetAllSlimMaps;
using COLOR::GetAllStandardMaps;
using COLOR::GetBlueStandardMaps;
using COLOR::GetCitiesStandardMaps;
using COLOR::GetColdStandardMaps;
using COLOR::GetGreenStandardMaps;
using COLOR::GetHeatStandardMaps;
using COLOR::GetMostlySequentialSlimMaps;
using COLOR::GetMostlySequentialStandardMaps;
using COLOR::GetOrangeStandardMaps;
using COLOR::GetPastelStandardMaps;
using COLOR::GetPurpleStandardMaps;
using COLOR::GetRedStandardMaps;
using COLOR::GetSeasonsStandardMaps;
using COLOR::GetSlightlyDivergingSlimMaps;
using COLOR::GetSlightlyDivergingStandardMaps;
using COLOR::GetYellowStandardMaps;
using COLOR::RandomColorMaps;
using UTILS::GetRandInRange;
using UTILS::NUM;

enum class GoomEffect
{
  DOTS0 = 0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  LINES1,
  LINES2,
  IFS,
  STARS,
  STARS_LOW,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _NUM // unused and must be last
};

using ColorMapFunc = std::function<std::shared_ptr<RandomColorMaps>()>;
using GoomEfectsColorMatchedMap = std::map<GoomEffect, ColorMapFunc>;

[[nodiscard]] static auto GetConstMatchedMap(const ColorMapFunc& func) -> GoomEfectsColorMatchedMap
{
  GoomEfectsColorMatchedMap colorMatchedMap{};

  for (size_t i = 0; i < NUM<GoomEffect>; ++i)
  {
    colorMatchedMap.emplace(static_cast<GoomEffect>(i), func);
  }

  return colorMatchedMap;
}

static const std::array<GoomEfectsColorMatchedMap, 15> GOOM_EFFECTS_COLOR_MATCHED_SETS{{
    {GetConstMatchedMap(GetRedStandardMaps)},
    {GetConstMatchedMap(GetGreenStandardMaps)},
    {GetConstMatchedMap(GetBlueStandardMaps)},
    {GetConstMatchedMap(GetYellowStandardMaps)},
    {GetConstMatchedMap(GetOrangeStandardMaps)},
    {GetConstMatchedMap(GetPurpleStandardMaps)},
    {
        {GoomEffect::DOTS0, GetAllStandardMaps},
        {GoomEffect::DOTS1, GetAllStandardMaps},
        {GoomEffect::DOTS2, GetAllStandardMaps},
        {GoomEffect::DOTS3, GetAllStandardMaps},
        {GoomEffect::DOTS4, GetAllStandardMaps},
        {GoomEffect::LINES1, GetAllStandardMaps},
        {GoomEffect::LINES2, GetAllStandardMaps},
        {GoomEffect::IFS, GetAllStandardMaps},
        {GoomEffect::STARS, GetAllStandardMaps},
        {GoomEffect::STARS_LOW, GetAllStandardMaps},
        {GoomEffect::TENTACLES, GetAllStandardMaps},
        {GoomEffect::TUBE, GetAllStandardMaps},
        {GoomEffect::TUBE_LOW, GetAllStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetMostlySequentialStandardMaps},
        {GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
        {GoomEffect::IFS, GetSlightlyDivergingStandardMaps},
        {GoomEffect::STARS, GetMostlySequentialSlimMaps},
        {GoomEffect::STARS_LOW, GetSlightlyDivergingStandardMaps},
        {GoomEffect::TENTACLES, GetSlightlyDivergingSlimMaps},
        {GoomEffect::TUBE, GetSlightlyDivergingStandardMaps},
        {GoomEffect::TUBE_LOW, GetSlightlyDivergingSlimMaps},
    },
    {
        {GoomEffect::DOTS0, GetOrangeStandardMaps},
        {GoomEffect::DOTS1, GetPurpleStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES2, GetSlightlyDivergingStandardMaps},
        {GoomEffect::IFS, GetSlightlyDivergingSlimMaps},
        {GoomEffect::STARS, GetHeatStandardMaps},
        {GoomEffect::STARS_LOW, GetAllSlimMaps},
        {GoomEffect::TENTACLES, GetYellowStandardMaps},
        {GoomEffect::TUBE, GetYellowStandardMaps},
        {GoomEffect::TUBE_LOW, GetBlueStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetOrangeStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetPastelStandardMaps},
        {GoomEffect::LINES1, GetAllSlimMaps},
        {GoomEffect::LINES2, GetBlueStandardMaps},
        {GoomEffect::IFS, GetColdStandardMaps},
        {GoomEffect::STARS, GetSlightlyDivergingSlimMaps},
        {GoomEffect::STARS_LOW, GetBlueStandardMaps},
        {GoomEffect::TENTACLES, GetMostlySequentialStandardMaps},
        {GoomEffect::TUBE, GetMostlySequentialStandardMaps},
        {GoomEffect::TUBE_LOW, GetHeatStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetPurpleStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetCitiesStandardMaps},
        {GoomEffect::STARS, GetBlueStandardMaps},
        {GoomEffect::STARS_LOW, GetMostlySequentialStandardMaps},
        {GoomEffect::TENTACLES, GetPurpleStandardMaps},
        {GoomEffect::TUBE, GetPurpleStandardMaps},
        {GoomEffect::TUBE_LOW, GetPastelStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetSlightlyDivergingSlimMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetMostlySequentialStandardMaps},
        {GoomEffect::TENTACLES, GetSeasonsStandardMaps},
        {GoomEffect::TUBE, GetSeasonsStandardMaps},
        {GoomEffect::TUBE_LOW, GetColdStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetHeatStandardMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetColdStandardMaps},
        {GoomEffect::TENTACLES, GetSeasonsStandardMaps},
        {GoomEffect::TUBE, GetSeasonsStandardMaps},
        {GoomEffect::TUBE_LOW, GetCitiesStandardMaps},
    },
    {
        {GoomEffect::DOTS0, GetRedStandardMaps},
        {GoomEffect::DOTS1, GetBlueStandardMaps},
        {GoomEffect::DOTS2, GetGreenStandardMaps},
        {GoomEffect::DOTS3, GetYellowStandardMaps},
        {GoomEffect::DOTS4, GetHeatStandardMaps},
        {GoomEffect::LINES1, GetSlightlyDivergingStandardMaps},
        {GoomEffect::LINES2, GetRedStandardMaps},
        {GoomEffect::IFS, GetPastelStandardMaps},
        {GoomEffect::STARS, GetPastelStandardMaps},
        {GoomEffect::STARS_LOW, GetAllMapsUnweighted},
        {GoomEffect::TENTACLES, GetGreenStandardMaps},
        {GoomEffect::TUBE, GetAllMapsUnweighted},
        {GoomEffect::TUBE_LOW, GetAllSlimMaps},
    },
    {
        {GoomEffect::DOTS0, GetPastelStandardMaps},
        {GoomEffect::DOTS1, GetPastelStandardMaps},
        {GoomEffect::DOTS2, GetPastelStandardMaps},
        {GoomEffect::DOTS3, GetPastelStandardMaps},
        {GoomEffect::DOTS4, GetPastelStandardMaps},
        {GoomEffect::LINES1, GetAllStandardMaps},
        {GoomEffect::LINES2, GetAllStandardMaps},
        {GoomEffect::IFS, GetRedStandardMaps},
        {GoomEffect::STARS, GetBlueStandardMaps},
        {GoomEffect::STARS_LOW, GetBlueStandardMaps},
        {GoomEffect::TENTACLES, GetYellowStandardMaps},
        {GoomEffect::TUBE, GetGreenStandardMaps},
        {GoomEffect::TUBE_LOW, GetHeatStandardMaps},
    },
}};

[[nodiscard]] inline auto GetNextColorMatchedMap() -> const GoomEfectsColorMatchedMap&
{
  return GOOM_EFFECTS_COLOR_MATCHED_SETS[GetRandInRange(0U,
                                                        GOOM_EFFECTS_COLOR_MATCHED_SETS.size())];
}

void GoomAllVisualFx::ChangeColorMaps()
{
  const GoomEfectsColorMatchedMap& colorMatchedMap = GetNextColorMatchedMap();

  m_goomDots_fx->SetWeightedColorMaps(0, colorMatchedMap.at(GoomEffect::DOTS0)());
  m_goomDots_fx->SetWeightedColorMaps(1, colorMatchedMap.at(GoomEffect::DOTS1)());
  m_goomDots_fx->SetWeightedColorMaps(2, colorMatchedMap.at(GoomEffect::DOTS2)());
  m_goomDots_fx->SetWeightedColorMaps(3, colorMatchedMap.at(GoomEffect::DOTS3)());
  m_goomDots_fx->SetWeightedColorMaps(4, colorMatchedMap.at(GoomEffect::DOTS4)());

  m_ifs_fx->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::IFS)());

  m_goomLine1->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::LINES1)());
  m_goomLine2->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::LINES2)());

  m_star_fx->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::STARS)());
  m_star_fx->SetWeightedLowColorMaps(colorMatchedMap.at(GoomEffect::STARS_LOW)());

  m_tentacles_fx->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::TENTACLES)());

  m_tube_fx->SetWeightedColorMaps(colorMatchedMap.at(GoomEffect::TUBE)());
  m_tube_fx->SetWeightedLowColorMaps(colorMatchedMap.at(GoomEffect::TUBE_LOW)());
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
