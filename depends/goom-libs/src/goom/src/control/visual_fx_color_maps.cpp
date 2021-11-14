#include "visual_fx_color_maps.h"

#include "color/random_colormaps.h"
#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"

#include <array>
#include <numeric>

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
using UTILS::IGoomRand;
using UTILS::NUM;
using UTILS::ToUType;

VisualFxColorMaps::VisualFxColorMaps(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    // clang-format off
    m_colorMatchedSets
    {
        m_goomRand,
        {
             {GetColorPairColorMatchedSet(GetRedStandardMaps,    GetGreenStandardMaps),    10},
             {GetColorPairColorMatchedSet(GetRedStandardMaps,    GetBlueStandardMaps),     10},
             {GetColorPairColorMatchedSet(GetYellowStandardMaps, GetBlueStandardMaps),     10},
             {GetColorPairColorMatchedSet(GetYellowStandardMaps, GetPurpleStandardMaps),   10},
             {GetColorPairColorMatchedSet(GetOrangeStandardMaps, GetGreenStandardMaps),    10},
             {GetColorPairColorMatchedSet(GetOrangeStandardMaps, GetPurpleStandardMaps),   10},
             {GetConstColorMatchedSet(GetAllStandardMaps),                                 20},
             {GetConstColorMatchedSet(GetHeatStandardMaps),                                15},
             {GetConstColorMatchedSet(GetColdStandardMaps),                                15},
             {GetConstColorMatchedSet(GetDivergingBlackStandardMaps),                      40},
             {GetColorMatchedSet1(),                                                       90},
             {GetColorMatchedSet2(),                                                       90},
             {GetColorMatchedSet3(),                                                       90},
             {GetColorMatchedSet4(),                                                       90},
             {GetColorMatchedSet5(),                                                       90},
             {GetColorMatchedSet6(),                                                       90},
             {GetColorMatchedSet7(),                                                       90},
             {GetColorMatchedSet8(),                                                       90},
        }
    },
    // clang-format on
    m_currentColorMatchedMap{&GetNextColorMatchedSet()}
{
}

auto VisualFxColorMaps::GetNextColorMatchedSet() -> const ColorMatchedSet&
{
  return m_colorMatchedSets.GetRandomWeighted();
}

inline auto VisualFxColorMaps::GetConstColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;
  matchedSet.fill(func);
  return matchedSet;
}

auto VisualFxColorMaps::GetColorPairColorMatchedSet(const ColorMapFunc& func1,
                                                    const ColorMapFunc& func2) -> ColorMatchedSet
{
  ColorMatchedSet matchedSet = GetConstColorMatchedSet(func1);
  std::array<size_t, NUM<GoomEffect>> indexes{};
  std::iota(begin(indexes), end(indexes), 0);
  m_goomRand.Shuffle(begin(indexes), end(indexes));
  for (size_t i = 0; i < indexes.size(); i += 2)
  {
    matchedSet.at(indexes.at(i)) = func2;
  }

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet1() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetMostlySequentialSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetSlightlyDivergingSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet2() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetOrangeStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetBlueStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet3() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetOrangeStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet4() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetPastelStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet5() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetColdStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet6() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetCitiesStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet7() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetAllMapsUnweighted;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetAllMapsUnweighted;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet8() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet;

  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedSet;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
