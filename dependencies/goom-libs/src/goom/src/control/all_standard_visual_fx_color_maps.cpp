//TODO split out into color map file
#include "all_standard_visual_fx.h"
#include "color/random_colormaps.h"
#include "goom/logging_control.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "utils/enumutils.h"
#include "visual_fx/flying_stars_fx.h"
#include "visual_fx/goom_dots_fx.h"
#include "visual_fx/ifs_dancers_fx.h"
#include "visual_fx/image_fx.h"
#include "visual_fx/lines_fx.h"
#include "visual_fx/tentacles_fx.h"
#include "visual_fx/tube_fx.h"

#include <array>
#include <functional>
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
using COLOR::GetDivergingBlackStandardMaps;
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
using UTILS::Logging;
using UTILS::NUM;
using UTILS::ToUType;

namespace
{

enum class GoomEffect
{
  DOTS0 = 0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  IFS,
  IMAGE,
  LINES1,
  LINES2,
  STARS,
  STARS_LOW,
  TENTACLES,
  TUBE,
  TUBE_LOW,
  _NUM // unused and must be last
};

using ColorMapFunc = std::function<std::shared_ptr<RandomColorMaps>()>;
using ColorMatchedMap = std::array<ColorMapFunc, NUM<GoomEffect>>;

[[nodiscard]] inline auto GetNextColorMatchedMap() noexcept -> const ColorMatchedMap&;
[[nodiscard]] inline auto GetColorMap(const ColorMatchedMap& colorMatchedMap, GoomEffect goomEffect)
    -> std::shared_ptr<RandomColorMaps>;

} // namespace

void AllStandardVisualFx::ChangeColorMaps()
{
  const ColorMatchedMap& colorMatchedMap = GetNextColorMatchedMap();

  m_goomDots_fx->SetWeightedColorMaps(0, GetColorMap(colorMatchedMap, GoomEffect::DOTS0));
  m_goomDots_fx->SetWeightedColorMaps(1, GetColorMap(colorMatchedMap, GoomEffect::DOTS1));
  m_goomDots_fx->SetWeightedColorMaps(2, GetColorMap(colorMatchedMap, GoomEffect::DOTS2));
  m_goomDots_fx->SetWeightedColorMaps(3, GetColorMap(colorMatchedMap, GoomEffect::DOTS3));
  m_goomDots_fx->SetWeightedColorMaps(4, GetColorMap(colorMatchedMap, GoomEffect::DOTS4));

  m_ifs_fx->SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::IFS));

  m_image_fx->SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::IMAGE));

  m_star_fx->SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::STARS));
  m_star_fx->SetWeightedLowColorMaps(GetColorMap(colorMatchedMap, GoomEffect::STARS_LOW));

  m_tentacles_fx->SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::TENTACLES));

  m_tube_fx->SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::TUBE));
  m_tube_fx->SetWeightedLowColorMaps(GetColorMap(colorMatchedMap, GoomEffect::TUBE_LOW));
}

void AllStandardVisualFx::ChangeLineColorMaps(VISUAL_FX::LinesFx& goomLine1,
                                              VISUAL_FX::LinesFx& goomLine2)
{
  const ColorMatchedMap& colorMatchedMap = GetNextColorMatchedMap();

  goomLine1.SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::LINES1));
  goomLine2.SetWeightedColorMaps(GetColorMap(colorMatchedMap, GoomEffect::LINES2));
}

namespace
{

[[nodiscard]] auto GetConstColorMatchedMap(const ColorMapFunc& func) noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;
  matchedMap.fill(func);
  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap1() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetMostlySequentialSlimMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetSlightlyDivergingSlimMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap2() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetOrangeStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetPurpleStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetHeatStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetAllSlimMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetBlueStandardMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap3() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetOrangeStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetColdStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetColdStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetAllSlimMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetMostlySequentialStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetMostlySequentialStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap4() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetPurpleStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetCitiesStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetCitiesStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetMostlySequentialStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetPurpleStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetPurpleStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetPastelStandardMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap5() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetSlightlyDivergingSlimMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetMostlySequentialStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetColdStandardMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap6() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetColdStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetCitiesStandardMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap7() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetAllMapsUnweighted;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetAllMapsUnweighted;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetAllSlimMaps;

  return matchedMap;
}

[[nodiscard]] auto GetColorMatchedMap8() noexcept -> ColorMatchedMap
{
  ColorMatchedMap matchedMap;

  matchedMap.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IFS)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::IMAGE)) = GetRedStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES1)) = GetAllStandardMaps;
  matchedMap.at(ToUType(GoomEffect::LINES2)) = GetAllStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::STARS_LOW)) = GetBlueStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE)) = GetGreenStandardMaps;
  matchedMap.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedMap;
}

constexpr size_t NUM_COLOR_MATCHED_SETS = 18;
using ColorMatchedSets = std::array<ColorMatchedMap, NUM_COLOR_MATCHED_SETS>;

[[nodiscard]] inline auto GetColorMatchedSets() noexcept -> const ColorMatchedSets&
{
  static const ColorMatchedSets s_COLOR_MATCHED_SETS{{
      {GetConstColorMatchedMap(GetRedStandardMaps)},
      {GetConstColorMatchedMap(GetGreenStandardMaps)},
      {GetConstColorMatchedMap(GetBlueStandardMaps)},
      {GetConstColorMatchedMap(GetYellowStandardMaps)},
      {GetConstColorMatchedMap(GetOrangeStandardMaps)},
      {GetConstColorMatchedMap(GetPurpleStandardMaps)},
      {GetConstColorMatchedMap(GetAllStandardMaps)},
      {GetConstColorMatchedMap(GetHeatStandardMaps)},
      {GetConstColorMatchedMap(GetColdStandardMaps)},
      {GetConstColorMatchedMap(GetDivergingBlackStandardMaps)},
      {GetColorMatchedMap1()},
      {GetColorMatchedMap2()},
      {GetColorMatchedMap3()},
      {GetColorMatchedMap4()},
      {GetColorMatchedMap5()},
      {GetColorMatchedMap6()},
      {GetColorMatchedMap7()},
      {GetColorMatchedMap8()},
  }};

  return s_COLOR_MATCHED_SETS;
}

[[nodiscard]] inline auto GetNextColorMatchedMap() noexcept -> const ColorMatchedMap&
{
  return GetColorMatchedSets()[GetRandInRange(0U, NUM_COLOR_MATCHED_SETS)];
}

[[nodiscard]] inline auto GetColorMap(const ColorMatchedMap& colorMatchedMap,
                                      const GoomEffect goomEffect)
    -> std::shared_ptr<RandomColorMaps>
{
  return colorMatchedMap[static_cast<size_t>(goomEffect)]();
}

} // namespace

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
