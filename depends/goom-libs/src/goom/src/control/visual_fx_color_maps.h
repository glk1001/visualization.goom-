#pragma once

#include "color/random_color_maps.h"
#include "goom_config.h"
#include "goom_effects.h"
#include "utils/math/goom_rand_base.h"
#include "visual_fx_color_matched_sets.h"

#include <memory>

namespace GOOM::CONTROL
{

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand);

  auto ChangeRandomColorMaps() -> void;

  [[nodiscard]] auto GetCurrentRandomColorMaps(GoomEffect goomEffect) const
      -> std::shared_ptr<const COLOR::RandomColorMaps>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  COLOR::RandomColorMapsGroups m_randomColorMapsGroups{m_goomRand};
  VisualFxColorMatchedSets m_visualFxColorMatchedSets{m_goomRand};
};

inline VisualFxColorMaps::VisualFxColorMaps(const UTILS::MATH::IGoomRand& goomRand)
  : m_goomRand{goomRand}
{
}

inline auto VisualFxColorMaps::ChangeRandomColorMaps() -> void
{
  m_visualFxColorMatchedSets.SetNextRandomColorMatchedSet();
}

inline auto VisualFxColorMaps::GetCurrentRandomColorMaps(const GoomEffect goomEffect) const
    -> std::shared_ptr<const COLOR::RandomColorMaps>
{
  if (static constexpr float PROB_COMPLETELY_RANDOM = 0.05F;
      m_goomRand.ProbabilityOf(PROB_COMPLETELY_RANDOM))
  {
    const auto randomGroup = static_cast<COLOR::RandomColorMapsGroups::Groups>(
        m_goomRand.GetRandInRange(0U, UTILS::NUM<COLOR::RandomColorMapsGroups::Groups>));
    return m_randomColorMapsGroups.MakeRandomColorMapsGroup(randomGroup);
  }

  return m_randomColorMapsGroups.MakeRandomColorMapsGroup(
      m_visualFxColorMatchedSets.GetCurrentRandomColorMapsGroup(goomEffect));
}

} // namespace GOOM::CONTROL
