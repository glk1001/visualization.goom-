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
import Goom.Utils.Math.GoomRandBase;
import Goom.VisualFx.IfsDancersFx;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;

namespace GOOM::VISUAL_FX::IFS
{

class Colorizer
{
public:
  Colorizer(const UTILS::MATH::IGoomRand& goomRand, PixelChannelType defaultAlpha);

  [[nodiscard]] auto GetWeightedColorMaps() const -> const COLOR::WeightedRandomColorMaps&;
  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& weightedColorMaps) -> void;

  auto InitColorMaps() -> void;

  auto GetColorMaps() const -> const COLOR::RandomColorMaps&;

  auto GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
  auto SetForcedColorMode(VISUAL_FX::IfsDancersFx::ColorMode val) -> void;
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
  const UTILS::MATH::IGoomRand* m_goomRand;

  COLOR::WeightedRandomColorMaps m_colorMaps;
  COLOR::ConstColorMapSharedPtr m_mixerColorMapPtr1         = nullptr;
  COLOR::ConstColorMapSharedPtr m_previousMixerColorMapPtr1 = nullptr;
  COLOR::ConstColorMapSharedPtr m_mixerColorMapPtr2         = nullptr;
  COLOR::ConstColorMapSharedPtr m_previousMixerColorMapPtr2 = nullptr;
  auto UpdateMixerMaps() -> void;
  [[nodiscard]] auto GetNextColorMapTypes() const noexcept
      -> const std::set<COLOR::WeightedRandomColorMaps::ColorMapTypes>&;
  mutable uint32_t m_countSinceColorMapChange              = 0;
  static constexpr uint32_t MIN_COLOR_MAP_CHANGE_COMPLETED = 500;
  static constexpr uint32_t MAX_COLOR_MAP_CHANGE_COMPLETED = 1000;
  uint32_t m_colorMapChangeCompleted                       = MIN_COLOR_MAP_CHANGE_COMPLETED;

  VISUAL_FX::IfsDancersFx::ColorMode m_colorMode = VISUAL_FX::IfsDancersFx::ColorMode::MAP_COLORS;
  VISUAL_FX::IfsDancersFx::ColorMode m_forcedColorMode  = VISUAL_FX::IfsDancersFx::ColorMode::_NULL;
  uint32_t m_maxHitCount                                = 0;
  float m_logMaxHitCount                                = 0.0;
  static constexpr float MIN_T_AWAY_FROM_BASE_COLOR     = 0.0F;
  static constexpr float MAX_T_AWAY_FROM_BASE_COLOR     = 0.4F;
  static constexpr float INITIAL_T_AWAY_FROM_BASE_COLOR = 0.0F;
  float m_tAwayFromBaseColor = INITIAL_T_AWAY_FROM_BASE_COLOR; // in [0, 1]
  UTILS::MATH::Weights<VISUAL_FX::IfsDancersFx::ColorMode> m_colorModeWeights;
  auto GetNextColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
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

inline auto Colorizer::GetColorMaps() const -> const COLOR::RandomColorMaps&
{
  return m_colorMaps;
}

inline auto Colorizer::GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode
{
  return m_colorMode;
}

inline auto Colorizer::SetForcedColorMode(const VISUAL_FX::IfsDancersFx::ColorMode val) -> void
{
  m_forcedColorMode = val;
}

inline auto Colorizer::SetMaxHitCount(const uint32_t val) -> void
{
  m_maxHitCount    = val;
  m_logMaxHitCount = std::log(static_cast<float>(m_maxHitCount));
}

inline auto Colorizer::GetWeightedColorMaps() const -> const COLOR::WeightedRandomColorMaps&
{
  return m_colorMaps;
}

using COLOR::ColorMaps;
using COLOR::GetUnweightedRandomColorMaps;
using COLOR::WeightedRandomColorMaps;
using UTILS::MATH::IGoomRand;
using VISUAL_FX::IfsDancersFx;

static constexpr auto MAP_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MAP_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto MIX_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MIX_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto REVERSE_MIX_COLORS_WEIGHT    = 20.0F;
static constexpr auto SINGLE_COLORS_WEIGHT         = 05.0F;
static constexpr auto SINE_MIX_COLORS_WEIGHT       = 05.0F;
static constexpr auto SINE_MAP_COLORS_WEIGHT       = 05.0F;

Colorizer::Colorizer(const IGoomRand& goomRand, const PixelChannelType defaultAlpha)
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
  return m_goomRand->ProbabilityOf(PROB_NO_EXTRA_COLOR_MAP_TYPES)
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

  m_colorMapChangeCompleted =
      m_goomRand->GetRandInRange(MIN_COLOR_MAP_CHANGE_COMPLETED, MAX_COLOR_MAP_CHANGE_COMPLETED);
  m_tAwayFromBaseColor =
      m_goomRand->GetRandInRange(MIN_T_AWAY_FROM_BASE_COLOR, MAX_T_AWAY_FROM_BASE_COLOR);
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

  static constexpr auto MIN_T_BASE_MIX = 0.3F;
  static constexpr auto MAX_T_BASE_MIX = 0.5F;
  return m_goomRand->GetRandInRange(MIN_T_BASE_MIX, MAX_T_BASE_MIX);
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
