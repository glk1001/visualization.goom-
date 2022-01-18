#include "visual_fx_color_maps.h"

#include "color/random_colormaps.h"
#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"

#include <array>
#include <numeric>

namespace GOOM::CONTROL
{

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
using COLOR::GetWesAndersonMaps;
using COLOR::GetYellowStandardMaps;
using UTILS::IGoomRand;
using UTILS::NUM;
using UTILS::ToUType;

auto VisualFxColorMaps::GetColorMatchedSetArray() const -> ColorMatchedSetArray
{
  ColorMatchedSetArray colorSets{};

  // clang-format off
  colorSets.at(ToUType(ColorMatchedSets::RED_GREEN_STANDARD_MAPS))             = GetColorPairColorMatchedSet(GetRedStandardMaps,    GetGreenStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::RED_BLUE_STANDARD_MAPS))              = GetColorPairColorMatchedSet(GetRedStandardMaps,    GetBlueStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::YELLOW_BLUE_STANDARD_MAPS))           = GetColorPairColorMatchedSet(GetYellowStandardMaps, GetBlueStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::YELLOW_PURPLE_STANDARD_MAPS))         = GetColorPairColorMatchedSet(GetYellowStandardMaps, GetPurpleStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::ORANGE_GREEN_STANDARD_MAPS))          = GetColorPairColorMatchedSet(GetOrangeStandardMaps, GetGreenStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::ORANGE_PURPLE_STANDARD_MAPS))         = GetColorPairColorMatchedSet(GetOrangeStandardMaps, GetPurpleStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::CONST_ALL_STANDARD_MAPS))             = GetConstColorMatchedSet(GetAllStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::CONST_HEAT_STANDARD_MAPS))            = GetConstColorMatchedSet(GetHeatStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::CONST_COLD_STANDARD_MAPS))            = GetConstColorMatchedSet(GetColdStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::CONST_DIVERGING_BLACK_STANDARD_MAPS)) = GetConstColorMatchedSet(GetDivergingBlackStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::CONST_WES_ANDERSON_MAPS))             = GetConstColorMatchedSet(GetWesAndersonMaps);
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET1))                  = GetColorMatchedSet1();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET2))                  = GetColorMatchedSet2();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET3))                  = GetColorMatchedSet3();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET4))                  = GetColorMatchedSet4();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET5))                  = GetColorMatchedSet5();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET6))                  = GetColorMatchedSet6();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET7))                  = GetColorMatchedSet7();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET8))                  = GetColorMatchedSet8();
  // clang-format on

  return colorSets;
}

// clang-format off
constexpr float RED_GREEN_STANDARD_MAPS_WEIGHT             =   5.0F;
constexpr float RED_BLUE_STANDARD_MAPS_WEIGHT              =   5.0F;
constexpr float YELLOW_BLUE_STANDARD_MAPS_WEIGHT           =   5.0F;
constexpr float YELLOW_PURPLE_STANDARD_MAPS_WEIGHT         =   5.0F;
constexpr float ORANGE_GREEN_STANDARD_MAPS_WEIGHT          =   5.0F;
constexpr float ORANGE_PURPLE_STANDARD_MAPS_WEIGHT         =   5.0F;
constexpr float CONST_ALL_STANDARD_MAPS_WEIGHT             =  20.0F;
constexpr float CONST_HEAT_STANDARD_MAPS_WEIGHT            =  35.0F;
constexpr float CONST_COLD_STANDARD_MAPS_WEIGHT            =  35.0F;
constexpr float CONST_DIVERGING_BLACK_STANDARD_MAPS_WEIGHT =  40.0F;
constexpr float CONST_WES_ANDERSON_MAPS_WEIGHT             =  40.0F;
constexpr float COLOR_MATCHED_SET1_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET2_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET3_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET4_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET5_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET6_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET7_WEIGHT                  =  90.0F;
constexpr float COLOR_MATCHED_SET8_WEIGHT                  =  90.0F;
// clang-format on

VisualFxColorMaps::VisualFxColorMaps(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_colorMatchedSets{GetColorMatchedSetArray()},
    // clang-format off
    m_colorMatchedSetWeights
    {
        m_goomRand,
        {
             {ColorMatchedSets::RED_GREEN_STANDARD_MAPS,             RED_GREEN_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::RED_BLUE_STANDARD_MAPS,              RED_BLUE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::YELLOW_BLUE_STANDARD_MAPS,           YELLOW_BLUE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::YELLOW_PURPLE_STANDARD_MAPS,         YELLOW_PURPLE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::ORANGE_GREEN_STANDARD_MAPS,          ORANGE_GREEN_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::ORANGE_PURPLE_STANDARD_MAPS,         ORANGE_PURPLE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::CONST_ALL_STANDARD_MAPS,             CONST_ALL_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::CONST_HEAT_STANDARD_MAPS,            CONST_HEAT_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::CONST_COLD_STANDARD_MAPS,            CONST_COLD_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::CONST_DIVERGING_BLACK_STANDARD_MAPS, CONST_DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::CONST_WES_ANDERSON_MAPS,             CONST_WES_ANDERSON_MAPS_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET1,                  COLOR_MATCHED_SET1_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET2,                  COLOR_MATCHED_SET2_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET3,                  COLOR_MATCHED_SET3_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET4,                  COLOR_MATCHED_SET4_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET5,                  COLOR_MATCHED_SET5_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET6,                  COLOR_MATCHED_SET6_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET7,                  COLOR_MATCHED_SET7_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET8,                  COLOR_MATCHED_SET8_WEIGHT},
        }
    },
    // clang-format on
    m_currentColorMatchedMap{&GetNextColorMatchedSet()}
{
}

auto VisualFxColorMaps::GetNextColorMatchedSet() const -> const ColorMatchedSet&
{
  return m_colorMatchedSets.at(static_cast<size_t>(m_colorMatchedSetWeights.GetRandomWeighted()));
}

// TODO: CLion is giving false positives for unreachable functions.
// For example 'GetConstColorMatchedSet'.
inline auto VisualFxColorMaps::GetConstColorMatchedSet(const ColorMapFunc& func) -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};
  matchedSet.fill(func);

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorPairColorMatchedSet(const ColorMapFunc& func1,
                                                    const ColorMapFunc& func2) const
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet = GetConstColorMatchedSet(func1);
  std::array<size_t, NUM<GoomEffect>> indexes{};
  std::iota(begin(indexes), end(indexes), 0);
  m_goomRand.Shuffle(begin(indexes), end(indexes));
  constexpr size_t INC_BY_2 = 2;
  for (size_t i = 0; i < NUM<GoomEffect>; i += INC_BY_2)
  {
    matchedSet.at(indexes.at(i)) = func2;
  }

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetAllSlimMaps;

  return matchedSet;
}

void VisualFxColorMaps::GetPrimaryColorDots(ColorMatchedSet& matchedSet)
{
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetPurpleStandardMaps;
}

auto VisualFxColorMaps::GetColorMatchedSet1() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetSlightlyDivergingStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetSlightlyDivergingSlimMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetOrangeStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetMostlySequentialStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetCitiesStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
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
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedSet;
}

} // namespace GOOM::CONTROL
