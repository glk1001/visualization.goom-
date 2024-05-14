module;

#include "goom/goom_config.h"

export module Goom.Control.GoomAllVisualFx:VisualFxColorMaps;

import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Control.GoomEffects;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import :VisualFxColorMatchedSets;
import :VisualFxWeightedColorMaps;

export namespace GOOM::CONTROL
{

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand);

  auto ChangeRandomColorMaps() -> void;

  [[nodiscard]] auto GetCurrentRandomColorMaps(GoomEffect goomEffect) const
      -> COLOR::WeightedRandomColorMaps;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  COLOR::RandomColorMapsGroups m_randomColorMapsGroups{*m_goomRand};
  VisualFxColorMatchedSets m_visualFxColorMatchedSets{*m_goomRand};
  VisualFxWeightedColorMaps m_visualFxWeightedColorMaps{*m_goomRand};
  [[nodiscard]] auto GetNextRandomColorMapsGroup(GoomEffect goomEffect) const
      -> COLOR::RandomColorMapsGroups::Groups;
  [[nodiscard]] auto GetNextCompletelyRandomColorMapsGroup() const
      -> COLOR::RandomColorMapsGroups::Groups;
};

inline VisualFxColorMaps::VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand)
  : m_goomRand{&goomRand}
{
}

inline auto VisualFxColorMaps::ChangeRandomColorMaps() -> void
{
  m_visualFxColorMatchedSets.SetNextRandomColorMatchedSet();
}

inline auto VisualFxColorMaps::GetCurrentRandomColorMaps(const GoomEffect goomEffect) const
    -> COLOR::WeightedRandomColorMaps
{
  return m_randomColorMapsGroups.GetWeightedRandomColorMapsForGroup(
      GetNextRandomColorMapsGroup(goomEffect));
}

inline auto VisualFxColorMaps::GetNextRandomColorMapsGroup(const GoomEffect goomEffect) const
    -> COLOR::RandomColorMapsGroups::Groups
{
  //  COLOR::RandomColorMapsGroups randomColorMapsGroups{m_goomRand};
  //  const auto group = COLOR::RandomColorMapsGroups::Groups::PURPLE_STANDARD_MAPS;
  //  return randomColorMapsGroups.MakeRandomColorMapsGroup(group);

  if (static constexpr auto PROB_COMPLETELY_RANDOM = 0.05F;
      m_goomRand->ProbabilityOf(PROB_COMPLETELY_RANDOM))
  {
    return GetNextCompletelyRandomColorMapsGroup();
  }
  if (static constexpr auto PROB_WEIGHTED_COLOR_MAPS = 0.25F;
      m_goomRand->ProbabilityOf(PROB_WEIGHTED_COLOR_MAPS))
  {
    return m_visualFxWeightedColorMaps.GetCurrentRandomColorMapsGroup(goomEffect);
  }

  return m_visualFxColorMatchedSets.GetCurrentRandomColorMapsGroup(goomEffect);
}

inline auto VisualFxColorMaps::GetNextCompletelyRandomColorMapsGroup() const
    -> COLOR::RandomColorMapsGroups::Groups
{
  return static_cast<COLOR::RandomColorMapsGroups::Groups>(
      m_goomRand->GetRandInRange(0U, UTILS::NUM<COLOR::RandomColorMapsGroups::Groups>));
}

} // namespace GOOM::CONTROL
