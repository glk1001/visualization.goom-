module;

#include <cmath>
#include <cstdint>
#include <memory>
#include <set>
#include <utility>

module Goom.VisualFx.IfsDancersFx:Colorizer;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Utils.Math.GoomRand;
import Goom.VisualFx.IfsDancersFx;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;

using GOOM::COLOR::ColorMaps;
using GOOM::COLOR::ConstColorMapSharedPtr;
using GOOM::COLOR::GetUnweightedRandomColorMaps;
using GOOM::COLOR::RandomColorMaps;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::Weights;

namespace GOOM::VISUAL_FX::IFS
{

class Colorizer
{
public:
  Colorizer(const GoomRand& goomRand, PixelChannelType defaultAlpha);

  [[nodiscard]] auto GetWeightedColorMaps() const -> const WeightedRandomColorMaps&;
  auto SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void;

  auto InitColorMaps() -> void;

  auto GetColorMaps() const -> const RandomColorMaps&;

  auto GetColorMode() const -> IfsDancersFx::ColorMode;
  auto SetForcedColorMode(IfsDancersFx::ColorMode val) -> void;
  auto ChangeColorMode() -> void;

  auto ChangeColorMaps() -> void;

  auto SetMaxHitCount(uint32_t val) -> void;

  struct MixProperties
  {
    float brightness;
    float tMix;
    float tX;
    float tY;
  };
  [[nodiscard]] auto GetMixedColor(const Pixel& baseColor,
                                   uint32_t hitCount,
                                   const MixProperties& mixProperties) const -> Pixel;

private:
  const GoomRand* m_goomRand;

  WeightedRandomColorMaps m_colorMaps;
  ConstColorMapSharedPtr m_mixerColorMapPtr1         = nullptr;
  ConstColorMapSharedPtr m_previousMixerColorMapPtr1 = nullptr;
  ConstColorMapSharedPtr m_mixerColorMapPtr2         = nullptr;
  ConstColorMapSharedPtr m_previousMixerColorMapPtr2 = nullptr;
  auto UpdateMixerMaps() -> void;
  [[nodiscard]] auto GetNextColorMapTypes() const noexcept
      -> const std::set<WeightedRandomColorMaps::ColorMapTypes>&;
  mutable uint32_t m_countSinceColorMapChange            = 0;
  static constexpr auto COLOR_MAP_CHANGE_COMPLETED_RANGE = NumberRange{500U, 1000U};
  uint32_t m_colorMapChangeCompleted = COLOR_MAP_CHANGE_COMPLETED_RANGE.min;

  IfsDancersFx::ColorMode m_colorMode                  = IfsDancersFx::ColorMode::MAP_COLORS;
  IfsDancersFx::ColorMode m_forcedColorMode            = IfsDancersFx::ColorMode::_NULL;
  uint32_t m_maxHitCount                               = 0;
  float m_logMaxHitCount                               = 0.0;
  static constexpr auto T_AWAY_FROM_BASE_COLOR_RANGE   = NumberRange{0.0F, 0.4F};
  static constexpr auto INITIAL_T_AWAY_FROM_BASE_COLOR = 0.0F;
  float m_tAwayFromBaseColor = INITIAL_T_AWAY_FROM_BASE_COLOR; // in [0, 1]
  Weights<IfsDancersFx::ColorMode> m_colorModeWeights;
  auto GetNextColorMode() const -> IfsDancersFx::ColorMode;
  [[nodiscard]] auto GetMixedColorInfo(const Pixel& baseColor,
                                       float logAlpha,
                                       const MixProperties& mixProperties) const
      -> std::pair<Pixel, float>;
  [[nodiscard]] auto GetNextMixerMapColor(float t, float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetSineMixColor(float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetMapColorsTBaseMix() const -> float;
  [[nodiscard]] auto GetFinalMixedColor(const Pixel& baseColor,
                                        float tBaseMix,
                                        const Pixel& mixColor) const -> Pixel;

  static constexpr float GAMMA = 2.2F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

inline auto Colorizer::GetColorMaps() const -> const RandomColorMaps&
{
  return m_colorMaps;
}

inline auto Colorizer::GetColorMode() const -> IfsDancersFx::ColorMode
{
  return m_colorMode;
}

inline auto Colorizer::SetForcedColorMode(const IfsDancersFx::ColorMode val) -> void
{
  m_forcedColorMode = val;
}

inline auto Colorizer::SetMaxHitCount(const uint32_t val) -> void
{
  m_maxHitCount    = val;
  m_logMaxHitCount = std::log(static_cast<float>(m_maxHitCount));
}

inline auto Colorizer::GetWeightedColorMaps() const -> const WeightedRandomColorMaps&
{
  return m_colorMaps;
}

static constexpr auto MAP_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MAP_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto MIX_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MIX_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto REVERSE_MIX_COLORS_WEIGHT    = 20.0F;
static constexpr auto SINGLE_COLORS_WEIGHT         = 05.0F;
static constexpr auto SINE_MIX_COLORS_WEIGHT       = 05.0F;
static constexpr auto SINE_MAP_COLORS_WEIGHT       = 05.0F;

Colorizer::Colorizer(const GoomRand& goomRand, const PixelChannelType defaultAlpha)
  : m_goomRand{&goomRand},
    m_colorMaps{GetUnweightedRandomColorMaps(*m_goomRand, defaultAlpha)},
    m_colorModeWeights{
        *m_goomRand,
        {
            { IfsDancersFx::ColorMode::MAP_COLORS,            MAP_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE, MEGA_MAP_COLOR_CHANGE_WEIGHT },
            { IfsDancersFx::ColorMode::MIX_COLORS,            MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE, MEGA_MIX_COLOR_CHANGE_WEIGHT },
            { IfsDancersFx::ColorMode::REVERSE_MIX_COLORS,    REVERSE_MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINGLE_COLORS,         SINGLE_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINE_MIX_COLORS,       SINE_MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINE_MAP_COLORS,       SINE_MAP_COLORS_WEIGHT },
        }
    }
{
}

auto Colorizer::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void
{
  m_colorMaps = weightedColorMaps;
}

auto Colorizer::InitColorMaps() -> void
{
  const auto& colorMapTypes = GetNextColorMapTypes();

  m_mixerColorMapPtr1 = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);
  m_mixerColorMapPtr2 = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);

  m_previousMixerColorMapPtr1 = m_mixerColorMapPtr1;
  m_previousMixerColorMapPtr2 = m_mixerColorMapPtr2;
}

auto Colorizer::UpdateMixerMaps() -> void
{
  const auto& colorMapTypes = GetNextColorMapTypes();

  m_previousMixerColorMapPtr1 = m_mixerColorMapPtr1;
  m_mixerColorMapPtr1         = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);

  m_previousMixerColorMapPtr2 = m_mixerColorMapPtr2;
  m_mixerColorMapPtr2         = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);
}

inline auto Colorizer::GetNextColorMapTypes() const noexcept
    -> const std::set<WeightedRandomColorMaps::ColorMapTypes>&
{
  static constexpr auto PROB_NO_EXTRA_COLOR_MAP_TYPES = 0.9F;
  return m_goomRand->ProbabilityOf<PROB_NO_EXTRA_COLOR_MAP_TYPES>()
             ? WeightedRandomColorMaps::GetNoColorMapsTypes()
             : WeightedRandomColorMaps::GetAllColorMapsTypes();
}

auto Colorizer::ChangeColorMode() -> void
{
  if (m_forcedColorMode != IfsDancersFx::ColorMode::_NULL)
  {
    m_colorMode = m_forcedColorMode;
  }
  else
  {
    m_colorMode = GetNextColorMode();
  }
}

inline auto Colorizer::GetNextColorMode() const -> IfsDancersFx::ColorMode
{
  return m_colorModeWeights.GetRandomWeighted();
}

auto Colorizer::ChangeColorMaps() -> void
{
  UpdateMixerMaps();

  m_colorMapChangeCompleted  = m_goomRand->GetRandInRange<COLOR_MAP_CHANGE_COMPLETED_RANGE>();
  m_tAwayFromBaseColor       = m_goomRand->GetRandInRange<T_AWAY_FROM_BASE_COLOR_RANGE>();
  m_countSinceColorMapChange = m_colorMapChangeCompleted;
}

auto Colorizer::GetMixedColor(const Pixel& baseColor,
                              const uint32_t hitCount,
                              const MixProperties& mixProperties) const -> Pixel
{
  const auto logAlpha =
      m_maxHitCount <= 1 ? 1.0F : (std::log(static_cast<float>(hitCount)) / m_logMaxHitCount);

  const auto [mixColor, tBaseMix] = GetMixedColorInfo(baseColor, logAlpha, mixProperties);

  return m_colorAdjust.GetAdjustment(mixProperties.brightness * logAlpha,
                                     GetFinalMixedColor(baseColor, tBaseMix, mixColor));
}

auto Colorizer::GetMixedColorInfo(const Pixel& baseColor,
                                  const float logAlpha,
                                  const MixProperties& mixProperties) const
    -> std::pair<Pixel, float>
{
  switch (m_colorMode)
  {
    case IfsDancersFx::ColorMode::MAP_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE:
      return {GetNextMixerMapColor(
                  mixProperties.brightness * logAlpha, mixProperties.tX, mixProperties.tY),
              GetMapColorsTBaseMix()};

    case IfsDancersFx::ColorMode::MIX_COLORS:
    case IfsDancersFx::ColorMode::REVERSE_MIX_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE:
      return {GetNextMixerMapColor(mixProperties.tMix, mixProperties.tX, mixProperties.tY),
              1.0F - m_tAwayFromBaseColor};

    case IfsDancersFx::ColorMode::SINGLE_COLORS:
      return {baseColor, 1.0F - m_tAwayFromBaseColor};

    case IfsDancersFx::ColorMode::SINE_MIX_COLORS:
    case IfsDancersFx::ColorMode::SINE_MAP_COLORS:
      return {GetSineMixColor(mixProperties.tX, mixProperties.tY), 1.0F - m_tAwayFromBaseColor};

    default:
      FailFast();
  }
}

auto Colorizer::GetNextMixerMapColor(const float t, const float tX, const float tY) const -> Pixel
{
  Expects(m_mixerColorMapPtr1 != nullptr);
  Expects(m_mixerColorMapPtr2 != nullptr);
  Expects(m_previousMixerColorMapPtr1 != nullptr);
  Expects(m_previousMixerColorMapPtr2 != nullptr);

  const auto nextColor = ColorMaps::GetColorMix(
      m_mixerColorMapPtr1->GetColor(tX), m_mixerColorMapPtr2->GetColor(tY), t);
  if (0 == m_countSinceColorMapChange)
  {
    return nextColor;
  }

  const auto tTransition = static_cast<float>(m_countSinceColorMapChange) /
                           static_cast<float>(m_colorMapChangeCompleted);
  --m_countSinceColorMapChange;
  const auto prevNextColor = ColorMaps::GetColorMix(
      m_previousMixerColorMapPtr1->GetColor(tX), m_previousMixerColorMapPtr2->GetColor(tY), t);
  return ColorMaps::GetColorMix(nextColor, prevNextColor, tTransition);
}

inline auto Colorizer::GetMapColorsTBaseMix() const -> float
{
  if (m_colorMode == IfsDancersFx::ColorMode::MAP_COLORS)
  {
    return 1.0F - m_tAwayFromBaseColor;
  }

  static constexpr auto T_BASE_MIX_RANGE = NumberRange{0.3F, 0.5F};
  return m_goomRand->GetRandInRange<T_BASE_MIX_RANGE>();
}

inline auto Colorizer::GetSineMixColor(const float tX, const float tY) const -> Pixel
{
  static constexpr auto INITIAL_FREQ = 20.0F;
  static constexpr auto T_MIX_FACTOR = 0.5F;
  static constexpr auto Z_STEP       = 0.1F;
  static const auto s_FREQ           = INITIAL_FREQ;
  static auto s_z                    = 0.0F;

  const auto mixColor =
      GetNextMixerMapColor(T_MIX_FACTOR * (1.0F + std::sin(s_FREQ * s_z)), tX, tY);

  s_z += Z_STEP;

  return mixColor;
}

inline auto Colorizer::GetFinalMixedColor(const Pixel& baseColor,
                                          const float tBaseMix,
                                          const Pixel& mixColor) const -> Pixel
{
  if (m_colorMode == IfsDancersFx::ColorMode::REVERSE_MIX_COLORS)
  {
    return ColorMaps::GetColorMix(mixColor, baseColor, tBaseMix);
  }

  return ColorMaps::GetColorMix(baseColor, mixColor, tBaseMix);
}

} // namespace GOOM::VISUAL_FX::IFS
