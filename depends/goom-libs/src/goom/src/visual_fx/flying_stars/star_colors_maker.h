#pragma once

#include "goom_config.h"
#include "star_colors.h"
#include "utils/math/goom_rand_base.h"

#include <memory>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColorsMaker
{
public:
  StarColorsMaker(const UTILS::MATH::IGoomRand& goomRand,
                  const StarColors::ColorMapsSet& colorMapsSet) noexcept;

  auto SetColorMapSet(const StarColors::ColorMapsSet& colorMapsSet) noexcept -> void;
  auto ChangeColorMode() noexcept -> void;

  [[nodiscard]] auto GetNewStarColors(float withinClusterT) const noexcept -> StarColors;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto COLOR_MODE_MIX_COLORS_WEIGHT         = 30.0F;
  static constexpr auto COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT = 15.0F;
  static constexpr auto COLOR_MODE_SINE_MIX_COLORS_WEIGHT    = 05.0F;
  UTILS::MATH::Weights<StarColors::ColorMode> m_colorModeWeights;

  static constexpr auto PROB_REVERSE_WITHIN_CLUSTER_MIX = 0.5F;
  static constexpr auto PROB_SIMILAR_LOW_COLORS         = 0.5F;
  std::shared_ptr<StarColors::ColorProperties> m_nextStarColorProperties;
};

inline StarColorsMaker::StarColorsMaker(const UTILS::MATH::IGoomRand& goomRand,
                                        const StarColors::ColorMapsSet& colorMapsSet) noexcept
  : m_goomRand{&goomRand},
    m_colorModeWeights{
        *m_goomRand,
        {
            { StarColors::ColorMode::MIX_COLORS,         COLOR_MODE_MIX_COLORS_WEIGHT },
            { StarColors::ColorMode::REVERSE_MIX_COLORS, COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT },
            { StarColors::ColorMode::SINE_MIX_COLORS,    COLOR_MODE_SINE_MIX_COLORS_WEIGHT },
        }
    },
    m_nextStarColorProperties{
        std::make_shared<StarColors::ColorProperties>(
            StarColors::ColorProperties{colorMapsSet,
                                        m_colorModeWeights.GetRandomWeighted(),
                                        m_goomRand->ProbabilityOf(PROB_REVERSE_WITHIN_CLUSTER_MIX),
                                        m_goomRand->ProbabilityOf(PROB_SIMILAR_LOW_COLORS)})
    }
{
  Expects(colorMapsSet.currentMainColorMapPtr != nullptr);
  Expects(colorMapsSet.currentLowColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantMainColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantLowColorMapPtr != nullptr);
}

inline auto StarColorsMaker::SetColorMapSet(const StarColors::ColorMapsSet& colorMapsSet) noexcept
    -> void
{
  Expects(colorMapsSet.currentMainColorMapPtr != nullptr);
  Expects(colorMapsSet.currentLowColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantMainColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantLowColorMapPtr != nullptr);

  const auto oldColorMode               = m_nextStarColorProperties->colorMode;
  const auto oldReverseWithinClusterMix = m_nextStarColorProperties->reverseWithinClusterMix;
  const auto oldSimilarLowColors        = m_nextStarColorProperties->similarLowColors;
  m_nextStarColorProperties =
      std::make_shared<StarColors::ColorProperties>(StarColors::ColorProperties{
          colorMapsSet, oldColorMode, oldReverseWithinClusterMix, oldSimilarLowColors});
}

inline auto StarColorsMaker::ChangeColorMode() noexcept -> void
{
  const auto& oldColorMapsSet = m_nextStarColorProperties->colorMapsSet;
  m_nextStarColorProperties   = std::make_shared<StarColors::ColorProperties>(
      StarColors::ColorProperties{oldColorMapsSet,
                                  m_colorModeWeights.GetRandomWeighted(),
                                  m_goomRand->ProbabilityOf(PROB_REVERSE_WITHIN_CLUSTER_MIX),
                                  m_goomRand->ProbabilityOf(PROB_SIMILAR_LOW_COLORS)});
}

inline auto StarColorsMaker::GetNewStarColors(const float withinClusterT) const noexcept
    -> StarColors
{
  return StarColors{m_nextStarColorProperties, withinClusterT};
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
