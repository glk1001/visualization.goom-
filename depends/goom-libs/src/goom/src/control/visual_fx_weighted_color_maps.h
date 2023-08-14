#pragma once

#include "color/random_color_maps_groups.h"
#include "goom_config.h"
#include "goom_effects.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

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
