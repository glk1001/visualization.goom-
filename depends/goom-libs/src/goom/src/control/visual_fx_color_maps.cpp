#include "visual_fx_color_maps.h"

#include "color/random_colormaps.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"

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
using UTILS::NUM;
using UTILS::ToUType;
using UTILS::MATH::IGoomRand;

auto VisualFxColorMaps::GetColorMatchedSetsArray() const -> ColorMatchedSetsArray
{
  ColorMatchedSetsArray colorSets{};

  // clang-format off
  colorSets.at(ToUType(ColorMatchedSets::RED_GREEN_STANDARD_MAPS))            = GetTwoGroupsColorMatchedSet(GetRedStandardMaps,    GetGreenStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::RED_BLUE_STANDARD_MAPS))             = GetTwoGroupsColorMatchedSet(GetRedStandardMaps,    GetBlueStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::YELLOW_BLUE_STANDARD_MAPS))          = GetTwoGroupsColorMatchedSet(GetYellowStandardMaps, GetBlueStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::YELLOW_PURPLE_STANDARD_MAPS))        = GetTwoGroupsColorMatchedSet(GetYellowStandardMaps, GetPurpleStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::ORANGE_GREEN_STANDARD_MAPS))         = GetTwoGroupsColorMatchedSet(GetOrangeStandardMaps, GetGreenStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::ORANGE_PURPLE_STANDARD_MAPS))        = GetTwoGroupsColorMatchedSet(GetOrangeStandardMaps, GetPurpleStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::ALL_ONLY_STANDARD_MAPS))             = GetOneGroupColorMatchedSet(GetAllStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::HEAT_ONLY_STANDARD_MAPS))            = GetOneGroupColorMatchedSet(GetHeatStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::COLD_ONLY_STANDARD_MAPS))            = GetOneGroupColorMatchedSet(GetColdStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::DIVERGING_ONLY_STANDARD_MAPS))       = GetOneGroupColorMatchedSet(GetSlightlyDivergingSlimMaps);
  colorSets.at(ToUType(ColorMatchedSets::DIVERGING_BLACK_ONLY_STANDARD_MAPS)) = GetOneGroupColorMatchedSet(GetDivergingBlackStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::WES_ANDERSON_ONLY_MAPS))             = GetOneGroupColorMatchedSet(GetWesAndersonMaps);
  colorSets.at(ToUType(ColorMatchedSets::PASTEL_ONLY_MAPS))                   = GetOneGroupColorMatchedSet(GetPastelStandardMaps);
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET1))                 = GetColorMatchedSet1();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET2))                 = GetColorMatchedSet2();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET3))                 = GetColorMatchedSet3();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET4))                 = GetColorMatchedSet4();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET5))                 = GetColorMatchedSet5();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET6))                 = GetColorMatchedSet6();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET7))                 = GetColorMatchedSet7();
  colorSets.at(ToUType(ColorMatchedSets::COLOR_MATCHED_SET8))                 = GetColorMatchedSet8();
  // clang-format on

  return colorSets;
}

// clang-format off
static constexpr float RED_GREEN_STANDARD_MAPS_WEIGHT            =   5.0F;
static constexpr float RED_BLUE_STANDARD_MAPS_WEIGHT             =   5.0F;
static constexpr float YELLOW_BLUE_STANDARD_MAPS_WEIGHT          =   5.0F;
static constexpr float YELLOW_PURPLE_STANDARD_MAPS_WEIGHT        =   5.0F;
static constexpr float ORANGE_GREEN_STANDARD_MAPS_WEIGHT         =   5.0F;
static constexpr float ORANGE_PURPLE_STANDARD_MAPS_WEIGHT        =   5.0F;
static constexpr float ALL_ONLY_STANDARD_MAPS_WEIGHT             =  20.0F;
static constexpr float HEAT_ONLY_STANDARD_MAPS_WEIGHT            =  35.0F;
static constexpr float COLD_ONLY_STANDARD_MAPS_WEIGHT            =  35.0F;
static constexpr float DIVERGING_ONLY_STANDARD_MAPS_WEIGHT       =  40.0F;
static constexpr float DIVERGING_BLACK_ONLY_STANDARD_MAPS_WEIGHT =  40.0F;
static constexpr float WES_ANDERSON_ONLY_MAPS_WEIGHT             =  40.0F;
static constexpr float PASTEL_ONLY_MAPS_WEIGHT                   =  40.0F;
static constexpr float COLOR_MATCHED_SET1_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET2_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET3_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET4_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET5_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET6_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET7_WEIGHT                 =  90.0F;
static constexpr float COLOR_MATCHED_SET8_WEIGHT                 =  90.0F;
// clang-format on

VisualFxColorMaps::VisualFxColorMaps(const IGoomRand& goomRand)
  : m_goomRand{goomRand},
    // clang-format off
    m_colorMatchedSetsWeights
    {
        m_goomRand,
        {
             {ColorMatchedSets::RED_GREEN_STANDARD_MAPS,            RED_GREEN_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::RED_BLUE_STANDARD_MAPS,             RED_BLUE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::YELLOW_BLUE_STANDARD_MAPS,          YELLOW_BLUE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::YELLOW_PURPLE_STANDARD_MAPS,        YELLOW_PURPLE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::ORANGE_GREEN_STANDARD_MAPS,         ORANGE_GREEN_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::ORANGE_PURPLE_STANDARD_MAPS,        ORANGE_PURPLE_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::ALL_ONLY_STANDARD_MAPS,             ALL_ONLY_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::HEAT_ONLY_STANDARD_MAPS,            HEAT_ONLY_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::COLD_ONLY_STANDARD_MAPS,            COLD_ONLY_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::DIVERGING_ONLY_STANDARD_MAPS,       DIVERGING_ONLY_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::DIVERGING_BLACK_ONLY_STANDARD_MAPS, DIVERGING_BLACK_ONLY_STANDARD_MAPS_WEIGHT},
             {ColorMatchedSets::WES_ANDERSON_ONLY_MAPS,             WES_ANDERSON_ONLY_MAPS_WEIGHT},
             {ColorMatchedSets::PASTEL_ONLY_MAPS,                   PASTEL_ONLY_MAPS_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET1,                 COLOR_MATCHED_SET1_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET2,                 COLOR_MATCHED_SET2_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET3,                 COLOR_MATCHED_SET3_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET4,                 COLOR_MATCHED_SET4_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET5,                 COLOR_MATCHED_SET5_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET6,                 COLOR_MATCHED_SET6_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET7,                 COLOR_MATCHED_SET7_WEIGHT},
             {ColorMatchedSets::COLOR_MATCHED_SET8,                 COLOR_MATCHED_SET8_WEIGHT},
        }
    }
// clang-format on
{
}

auto VisualFxColorMaps::GetNextColorMatchedSet() const -> const ColorMatchedSet&
{
  return m_colorMatchedSets.at(static_cast<size_t>(m_colorMatchedSetsWeights.GetRandomWeighted()));
}

// TODO: CLion is giving false positives for unreachable functions.
// For example 'GetConstColorMatchedSet'.
auto VisualFxColorMaps::GetOneGroupColorMatchedSet(const ColorMapsFunc& func) -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};
  matchedSet.fill(func);

  GetPrimaryColorDots(matchedSet);
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetTwoGroupsColorMatchedSet(const ColorMapsFunc& func1,
                                                    const ColorMapsFunc& func2) const
    -> ColorMatchedSet
{
  ColorMatchedSet matchedSet = GetOneGroupColorMatchedSet(func1);
  std::array<size_t, NUM<GoomEffect>> indexes{};
  std::iota(begin(indexes), end(indexes), 0);
  m_goomRand.Shuffle(begin(indexes), end(indexes));
  static constexpr size_t INC_BY_2 = 2;
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

auto VisualFxColorMaps::GetPrimaryColorDots(ColorMatchedSet& matchedSet) -> void
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
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetMostlySequentialSlimMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetMostlySequentialSlimMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetColdStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetMostlySequentialSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetSlightlyDivergingSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet2() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetOrangeStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetColdStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetColdStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetBlueStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet3() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetOrangeStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetDivergingBlackStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetSlightlyDivergingStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetHeatStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet4() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetYellowStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetColdStandardMaps;
  **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetCitiesStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetDivergingBlackStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetAllSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetPastelStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet5() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetDivergingBlackStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetDivergingBlackStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetWesAndersonMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetMostlySequentialStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetDivergingBlackStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetColdStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet6() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetHeatStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetColdStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetSeasonsStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetCitiesStandardMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet7() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetSlightlyDivergingStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetGreenStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetColdStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetPastelStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetAllMapsUnweighted;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetMostlySequentialSlimMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetColdStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetAllMapsUnweighted;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetAllSlimMaps;

  return matchedSet;
}

auto VisualFxColorMaps::GetColorMatchedSet8() -> ColorMatchedSet
{
  ColorMatchedSet matchedSet{nullptr};

  matchedSet.at(ToUType(GoomEffect::CIRCLES)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::CIRCLES_LOW)) = GetSlightlyDivergingSlimMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS0)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS1)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS2)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS3)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::DOTS4)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IFS)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::IMAGE)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES1)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::LINES2)) = GetAllStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_MAIN)) = GetWesAndersonMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_LOW)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES0_INNER)) = GetSlightlyDivergingStandardMaps;
  /**
  matchedSet.at(ToUType(GoomEffect::SHAPES1_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_LOW)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES1_INNER)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_MAIN)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_LOW)) = GetHeatStandardMaps;
  matchedSet.at(ToUType(GoomEffect::SHAPES2_INNER)) = GetHeatStandardMaps;
   **/
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FIREWORKS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FIREWORKS)) = GetBlueStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_RAIN)) = GetRedStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_RAIN)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_MAIN_FOUNTAIN)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::STARS_LOW_FOUNTAIN)) = GetPurpleStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TENTACLES)) = GetYellowStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE)) = GetGreenStandardMaps;
  matchedSet.at(ToUType(GoomEffect::TUBE_LOW)) = GetHeatStandardMaps;

  return matchedSet;
}

} // namespace GOOM::CONTROL
