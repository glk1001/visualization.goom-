module;

#include <memory>

module Goom.VisualFx.FlyingStarsFx:StarColorsMaker;

import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import :StarColors;

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColorsMaker
{
public:
  StarColorsMaker(const UTILS::MATH::GoomRand& goomRand,
                  const StarColors::ColorMapsSet& colorMapsSet) noexcept;

  auto SetColorMapSet(const StarColors::ColorMapsSet& colorMapsSet) noexcept -> void;
  auto ChangeColorMode() noexcept -> void;

  [[nodiscard]] auto GetNewStarColors(float withinClusterT) const noexcept -> StarColors;

private:
  const UTILS::MATH::GoomRand* m_goomRand;

  static constexpr auto COLOR_MODE_MIX_COLORS_WEIGHT         = 30.0F;
  static constexpr auto COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT = 15.0F;
  static constexpr auto COLOR_MODE_SINE_MIX_COLORS_WEIGHT    = 05.0F;
  UTILS::MATH::Weights<StarColors::ColorMode> m_colorModeWeights;

  static constexpr auto PROB_REVERSE_WITHIN_CLUSTER_MIX = 0.5F;
  static constexpr auto PROB_SIMILAR_LOW_COLORS         = 0.5F;
  StarColors::ColorProperties m_nextStarColorProperties;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

StarColorsMaker::StarColorsMaker(const UTILS::MATH::GoomRand& goomRand,
                                 const StarColors::ColorMapsSet& colorMapsSet) noexcept
  : m_goomRand{&goomRand},
    m_colorModeWeights{
        *m_goomRand,
        {
            { .key=StarColors::ColorMode::MIX_COLORS,         .weight=COLOR_MODE_MIX_COLORS_WEIGHT },
            { .key=StarColors::ColorMode::REVERSE_MIX_COLORS, .weight=COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT },
            { .key=StarColors::ColorMode::SINE_MIX_COLORS,    .weight=COLOR_MODE_SINE_MIX_COLORS_WEIGHT },
        }
    },
    m_nextStarColorProperties{
            .colorMapsSet =colorMapsSet,
            .colorMode = m_colorModeWeights.GetRandomWeighted(),
            .reverseWithinClusterMix = m_goomRand->ProbabilityOf<PROB_REVERSE_WITHIN_CLUSTER_MIX>(),
            .similarLowColors = m_goomRand->ProbabilityOf<PROB_SIMILAR_LOW_COLORS>()
    }
{
  Expects(colorMapsSet.currentMainColorMapPtr != nullptr);
  Expects(colorMapsSet.currentLowColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantMainColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantLowColorMapPtr != nullptr);
}

auto StarColorsMaker::SetColorMapSet(const StarColors::ColorMapsSet& colorMapsSet) noexcept -> void
{
  Expects(colorMapsSet.currentMainColorMapPtr != nullptr);
  Expects(colorMapsSet.currentLowColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantMainColorMapPtr != nullptr);
  Expects(colorMapsSet.dominantLowColorMapPtr != nullptr);

  const auto oldColorMode               = m_nextStarColorProperties.colorMode;
  const auto oldReverseWithinClusterMix = m_nextStarColorProperties.reverseWithinClusterMix;
  const auto oldSimilarLowColors        = m_nextStarColorProperties.similarLowColors;
  m_nextStarColorProperties =
      StarColors::ColorProperties{.colorMapsSet            = colorMapsSet,
                                  .colorMode               = oldColorMode,
                                  .reverseWithinClusterMix = oldReverseWithinClusterMix,
                                  .similarLowColors        = oldSimilarLowColors};
}

auto StarColorsMaker::ChangeColorMode() noexcept -> void
{
  const auto& oldColorMapsSet = m_nextStarColorProperties.colorMapsSet;
  m_nextStarColorProperties   = StarColors::ColorProperties{
        .colorMapsSet            = oldColorMapsSet,
        .colorMode               = m_colorModeWeights.GetRandomWeighted(),
        .reverseWithinClusterMix = m_goomRand->ProbabilityOf<PROB_REVERSE_WITHIN_CLUSTER_MIX>(),
        .similarLowColors        = m_goomRand->ProbabilityOf<PROB_SIMILAR_LOW_COLORS>()};
}

inline auto StarColorsMaker::GetNewStarColors(const float withinClusterT) const noexcept
    -> StarColors
{
  return StarColors{m_nextStarColorProperties, withinClusterT};
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
