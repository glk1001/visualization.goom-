#include "visual_fx_weighted_color_maps.h"

#include "utils/math/goom_rand_base.h"

namespace GOOM::CONTROL
{

using COLOR::RandomColorMapsGroups;
using UTILS::MATH::IGoomRand;

using Groups = RandomColorMapsGroups::Groups;

VisualFxWeightedColorMaps::VisualFxWeightedColorMaps(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_goomEffectsWeightedColorMaps{
        {GoomEffect::CIRCLES_MAIN, GetCirclesMainWeightedGroups(m_goomRand)},
        {GoomEffect::CIRCLES_LOW, GetCirclesLowWeightedGroups(m_goomRand)},
        {GoomEffect::DOTS0, GetDots0WeightedGroups(m_goomRand)},
        {GoomEffect::DOTS1, GetDots1WeightedGroups(m_goomRand)},
        {GoomEffect::DOTS2, GetDots2WeightedGroups(m_goomRand)},
        {GoomEffect::DOTS3, GetDots3WeightedGroups(m_goomRand)},
        {GoomEffect::DOTS4, GetDots4WeightedGroups(m_goomRand)},
        {GoomEffect::IFS, GetIfsWeightedGroups(m_goomRand)},
        {GoomEffect::IMAGE, GetImageWeightedGroups(m_goomRand)},
        {GoomEffect::LINES1, GetLines1WeightedGroups(m_goomRand)},
        {GoomEffect::LINES2, GetLines2WeightedGroups(m_goomRand)},
        {GoomEffect::SHAPES_MAIN, GetShapesMainWeightedGroups(m_goomRand)},
        {GoomEffect::SHAPES_LOW, GetShapesLowWeightedGroups(m_goomRand)},
        {GoomEffect::SHAPES_INNER, GetShapesInnerWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_MAIN_FIREWORKS, GetStarsMainFireworksWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_LOW_FIREWORKS, GetStarsLowFireworksWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_MAIN_RAIN, GetStarsMainRainWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_LOW_RAIN, GetStarsLowRainWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_MAIN_FOUNTAIN, GetStarsMainFountainWeightedGroups(m_goomRand)},
        {GoomEffect::STARS_LOW_FOUNTAIN, GetStarsLowFountainWeightedGroups(m_goomRand)},
        {GoomEffect::TENTACLES, GetTentaclesWeightedGroups(m_goomRand)},
        {GoomEffect::TUBE_MAIN, GetTubesMainWeightedGroups(m_goomRand)},
        {GoomEffect::TUBE_LOW, GetTubesLowWeightedGroups(m_goomRand)},
    }
{
}

auto VisualFxWeightedColorMaps::GetCirclesMainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetCirclesLowWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots0WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots1WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots2WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots3WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots4WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetIfsWeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetImageWeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =   0.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =   0.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =   0.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =   0.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =   0.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =   0.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLines1WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLines2WeightedGroups(const IGoomRand& goomRand) -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesMainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesLowWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesInnerWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainFireworksWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowFireworksWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainRainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowRainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainFountainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowFountainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTubesMainWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTubesLowWeightedGroups(const IGoomRand& goomRand)
    -> WeightedGroups
{
  // clang-format off
  static constexpr float ALL_MAPS_UNWEIGHTED_WEIGHT              =   5.0F;
  static constexpr float ALL_STANDARD_MAPS_WEIGHT                =   5.0F;
  static constexpr float ALL_SLIM_MAPS_WEIGHT                    =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  =   5.0F;
  static constexpr float MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT =   5.0F;
  static constexpr float SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     =  20.0F;
  static constexpr float DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    =  35.0F;
  static constexpr float RED_STANDARD_MAPS_WEIGHT                =  35.0F;
  static constexpr float GREEN_STANDARD_MAPS_WEIGHT              =  40.0F;
  static constexpr float BLUE_STANDARD_MAPS_WEIGHT               =  40.0F;
  static constexpr float YELLOW_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float ORANGE_STANDARD_MAPS_WEIGHT             =  40.0F;
  static constexpr float PURPLE_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float CITIES_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float SEASONS_STANDARD_MAPS_WEIGHT            =  90.0F;
  static constexpr float HEAT_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float COLD_STANDARD_MAPS_WEIGHT               =  90.0F;
  static constexpr float PASTEL_STANDARD_MAPS_WEIGHT             =  90.0F;
  static constexpr float WES_ANDERSON_MAPS_WEIGHT                =  90.0F;
  // clang-format on

  return {
      goomRand,
      {
        {Groups::ALL_MAPS_UNWEIGHTED, ALL_MAPS_UNWEIGHTED_WEIGHT},
        {Groups::ALL_STANDARD_MAPS, ALL_STANDARD_MAPS_WEIGHT},
        {Groups::ALL_SLIM_MAPS, ALL_SLIM_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_STANDARD_MAPS, MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT},
        {Groups::MOSTLY_SEQUENTIAL_SLIM_MAPS, MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_STANDARD_MAPS, SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT},
        {Groups::SLIGHTLY_DIVERGING_SLIM_MAPS, SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT},
        {Groups::DIVERGING_BLACK_STANDARD_MAPS, DIVERGING_BLACK_STANDARD_MAPS_WEIGHT},
        {Groups::RED_STANDARD_MAPS, RED_STANDARD_MAPS_WEIGHT},
        {Groups::GREEN_STANDARD_MAPS, GREEN_STANDARD_MAPS_WEIGHT},
        {Groups::BLUE_STANDARD_MAPS, BLUE_STANDARD_MAPS_WEIGHT},
        {Groups::YELLOW_STANDARD_MAPS, YELLOW_STANDARD_MAPS_WEIGHT},
        {Groups::ORANGE_STANDARD_MAPS, ORANGE_STANDARD_MAPS_WEIGHT},
        {Groups::PURPLE_STANDARD_MAPS, PURPLE_STANDARD_MAPS_WEIGHT},
        {Groups::CITIES_STANDARD_MAPS, CITIES_STANDARD_MAPS_WEIGHT},
        {Groups::SEASONS_STANDARD_MAPS, SEASONS_STANDARD_MAPS_WEIGHT},
        {Groups::HEAT_STANDARD_MAPS, HEAT_STANDARD_MAPS_WEIGHT},
        {Groups::COLD_STANDARD_MAPS, COLD_STANDARD_MAPS_WEIGHT},
        {Groups::PASTEL_STANDARD_MAPS, PASTEL_STANDARD_MAPS_WEIGHT},
        {Groups::WES_ANDERSON_MAPS, WES_ANDERSON_MAPS_WEIGHT},
        }
  };
}

} // namespace GOOM::CONTROL
