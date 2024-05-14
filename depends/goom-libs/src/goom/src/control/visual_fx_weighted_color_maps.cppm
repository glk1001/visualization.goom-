module;

#include "color/random_color_maps_groups.h"
#include "goom/goom_config.h"

module Goom.Control.GoomAllVisualFx:VisualFxWeightedColorMaps;

import Goom.Control.GoomEffects;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::CONTROL
{

class VisualFxWeightedColorMaps
{
public:
  explicit VisualFxWeightedColorMaps(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetCurrentRandomColorMapsGroup(GoomEffect goomEffect) const noexcept
      -> COLOR::RandomColorMapsGroups::Groups;

private:
  using WeightedGroups = UTILS::MATH::Weights<COLOR::RandomColorMapsGroups::Groups>;
  UTILS::RuntimeEnumMap<GoomEffect, WeightedGroups> m_goomEffectsWeightedColorMaps;

  [[nodiscard]] static auto GetCirclesMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetCirclesLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots0Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots1Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots2Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots3Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots4Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetIfsGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetImageGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetLines1Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetLines2Groups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetLSystemMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetLSystemLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetParticlesMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetParticlesLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetRaindropsMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetRaindropsLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesInnerGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFireworksGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFireworksGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainRainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowRainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFountainGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFountainGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesDominantMainGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesDominantLowGroups(
      const UTILS::MATH::IGoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTubesMainGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTubesLowGroups(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> WeightedGroups;
};

inline auto VisualFxWeightedColorMaps::GetCurrentRandomColorMapsGroup(
    const GoomEffect goomEffect) const noexcept -> COLOR::RandomColorMapsGroups::Groups
{
  return m_goomEffectsWeightedColorMaps[goomEffect].GetRandomWeighted();
}

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

using COLOR::RandomColorMapsGroups;
using UTILS::MATH::IGoomRand;
using Groups = RandomColorMapsGroups::Groups;

VisualFxWeightedColorMaps::VisualFxWeightedColorMaps(const IGoomRand& goomRand) noexcept
  : m_goomEffectsWeightedColorMaps{{{
        {GoomEffect::CIRCLES_MAIN, GetCirclesMainGroups(goomRand)},
        {GoomEffect::CIRCLES_LOW, GetCirclesLowGroups(goomRand)},
        {GoomEffect::DOTS0, GetDots0Groups(goomRand)},
        {GoomEffect::DOTS1, GetDots1Groups(goomRand)},
        {GoomEffect::DOTS2, GetDots2Groups(goomRand)},
        {GoomEffect::DOTS3, GetDots3Groups(goomRand)},
        {GoomEffect::DOTS4, GetDots4Groups(goomRand)},
        {GoomEffect::IFS, GetIfsGroups(goomRand)},
        {GoomEffect::IMAGE, GetImageGroups(goomRand)},
        {GoomEffect::L_SYSTEM_MAIN, GetLSystemMainGroups(goomRand)},
        {GoomEffect::L_SYSTEM_LOW, GetLSystemLowGroups(goomRand)},
        {GoomEffect::LINES1, GetLines1Groups(goomRand)},
        {GoomEffect::LINES2, GetLines2Groups(goomRand)},
        {GoomEffect::PARTICLES_MAIN, GetParticlesMainGroups(goomRand)},
        {GoomEffect::PARTICLES_LOW, GetParticlesLowGroups(goomRand)},
        {GoomEffect::RAINDROPS_MAIN, GetRaindropsMainGroups(goomRand)},
        {GoomEffect::RAINDROPS_LOW, GetRaindropsLowGroups(goomRand)},
        {GoomEffect::SHAPES_MAIN, GetShapesMainGroups(goomRand)},
        {GoomEffect::SHAPES_LOW, GetShapesLowGroups(goomRand)},
        {GoomEffect::SHAPES_INNER, GetShapesInnerGroups(goomRand)},
        {GoomEffect::STARS_MAIN_FIREWORKS, GetStarsMainFireworksGroups(goomRand)},
        {GoomEffect::STARS_LOW_FIREWORKS, GetStarsLowFireworksGroups(goomRand)},
        {GoomEffect::STARS_MAIN_RAIN, GetStarsMainRainGroups(goomRand)},
        {GoomEffect::STARS_LOW_RAIN, GetStarsLowRainGroups(goomRand)},
        {GoomEffect::STARS_MAIN_FOUNTAIN, GetStarsMainFountainGroups(goomRand)},
        {GoomEffect::STARS_LOW_FOUNTAIN, GetStarsLowFountainGroups(goomRand)},
        {GoomEffect::TENTACLES_DOMINANT_MAIN, GetTentaclesDominantMainGroups(goomRand)},
        {GoomEffect::TENTACLES_DOMINANT_LOW, GetTentaclesDominantLowGroups(goomRand)},
        {GoomEffect::TENTACLES_MAIN, GetTentaclesMainGroups(goomRand)},
        {GoomEffect::TENTACLES_LOW, GetTentaclesLowGroups(goomRand)},
        {GoomEffect::TUBE_MAIN, GetTubesMainGroups(goomRand)},
        {GoomEffect::TUBE_LOW, GetTubesLowGroups(goomRand)},
    }}}
{
}

auto VisualFxWeightedColorMaps::GetCirclesMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetCirclesLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetDots0Groups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetDots1Groups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetDots2Groups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetDots3Groups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetDots4Groups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetIfsGroups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetImageGroups(const IGoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetLines1Groups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetLines2Groups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetLSystemMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetLSystemLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetParticlesMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetRaindropsMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetParticlesLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetRaindropsLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetShapesMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetShapesLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetShapesInnerGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsMainFireworksGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsLowFireworksGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsMainRainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsLowRainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsMainFountainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetStarsLowFountainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetTentaclesDominantMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetTentaclesDominantLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetTentaclesMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
      // clang-format on
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetTubesMainGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

auto VisualFxWeightedColorMaps::GetTubesLowGroups(const IGoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WEIGHT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WEIGHT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WEIGHT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WEIGHT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WEIGHT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WEIGHT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WEIGHT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WEIGHT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WEIGHT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WEIGHT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WEIGHT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WEIGHT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WEIGHT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WEIGHT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WEIGHT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WEIGHT                = 90.0F;

  // clang-format off
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
  // clang-format on
}

} // namespace GOOM::CONTROL
