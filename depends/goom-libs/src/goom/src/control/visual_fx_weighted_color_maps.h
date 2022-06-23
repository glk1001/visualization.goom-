#pragma once

#include "color/random_color_maps_groups.h"
#include "goom_config.h"
#include "goom_effects.h"
#include "utils/math/goom_rand_base.h"

#include <map>
#include <memory>

namespace GOOM::CONTROL
{

class VisualFxWeightedColorMaps
{
public:
  explicit VisualFxWeightedColorMaps(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetCurrentRandomColorMapsGroup(GoomEffect goomEffect) const noexcept
      -> COLOR::RandomColorMapsGroups::Groups;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;

  using WeightedGroups = UTILS::MATH::Weights<COLOR::RandomColorMapsGroups::Groups>;
  const std::map<GoomEffect, WeightedGroups> m_goomEffectsWeightedColorMaps;

  [[nodiscard]] static auto GetCirclesMainWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetCirclesLowWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots0WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots1WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots2WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots3WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots4WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetIfsWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetImageWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetLines1WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetLines2WeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesMainWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesLowWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesInnerWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFireworksWeightedGroups(
      const UTILS::MATH::IGoomRand& goomRand) -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFireworksWeightedGroups(
      const UTILS::MATH::IGoomRand& goomRand) -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainRainWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowRainWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFountainWeightedGroups(
      const UTILS::MATH::IGoomRand& goomRand) -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFountainWeightedGroups(
      const UTILS::MATH::IGoomRand& goomRand) -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetTubesMainWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
  [[nodiscard]] static auto GetTubesLowWeightedGroups(const UTILS::MATH::IGoomRand& goomRand)
      -> WeightedGroups;
};

inline auto VisualFxWeightedColorMaps::GetCurrentRandomColorMapsGroup(
    GoomEffect goomEffect) const noexcept -> COLOR::RandomColorMapsGroups::Groups
{
  return m_goomEffectsWeightedColorMaps.at(goomEffect).GetRandomWeighted();
}

} // namespace GOOM::CONTROL
