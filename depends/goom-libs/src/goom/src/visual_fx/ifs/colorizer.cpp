#include "colorizer.h"

#include "color/color_maps.h"
#include "color/random_color_maps.h"

#include <cmath>

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::IColorMap;
using COLOR::MakeSharedSlightlyDivergingStandardMaps;
using COLOR::RandomColorMaps;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::MATH::IGoomRand;
using VISUAL_FX::IfsDancersFx;

// clang-format off
static constexpr float MAP_COLORS_WEIGHT            = 20.0F;
static constexpr float MEGA_MAP_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr float MIX_COLORS_WEIGHT            = 20.0F;
static constexpr float MEGA_MIX_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr float REVERSE_MIX_COLORS_WEIGHT    = 20.0F;
static constexpr float SINGLE_COLORS_WEIGHT         =  5.0F;
static constexpr float SINE_MIX_COLORS_WEIGHT       =  5.0F;
static constexpr float SINE_MAP_COLORS_WEIGHT       =  5.0F;
// clang-format on

Colorizer::Colorizer(const IGoomRand& goomRand)
  : m_goomRand{goomRand},
    m_colorMaps{MakeSharedSlightlyDivergingStandardMaps(m_goomRand)},
    m_mixerMap1Id{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    m_mixerMap2Id{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    m_colorModeWeights{
        m_goomRand,
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
  UpdateMixerMaps();
}

auto Colorizer::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps>& weightedColorMaps) -> void
{
  m_colorMaps = weightedColorMaps;

  UpdateMixerMaps();
}

auto Colorizer::UpdateMixerMaps() -> void
{
  static constexpr float PROB_NO_EXTRA_COLOR_MAP_TYPES = 0.9F;
  const std::set<RandomColorMaps::ColorMapTypes>& colorMapTypes =
      m_goomRand.ProbabilityOf(PROB_NO_EXTRA_COLOR_MAP_TYPES)
          ? RandomColorMaps::NO_COLOR_MAP_TYPES
          : RandomColorMaps::ALL_COLOR_MAP_TYPES;

  m_colorMapsManager.UpdateColorMapInfo(m_mixerMap1Id, {m_colorMaps, colorMapTypes});
  m_prevMixerMap1 = m_colorMapsManager.GetColorMapPtr(m_mixerMap1Id);

  m_colorMapsManager.UpdateColorMapInfo(m_mixerMap2Id, {m_colorMaps, colorMapTypes});
  m_prevMixerMap2 = m_colorMapsManager.GetColorMapPtr(m_mixerMap2Id);
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
  m_prevMixerMap1 = m_colorMapsManager.GetColorMapPtr(m_mixerMap1Id);
  m_prevMixerMap2 = m_colorMapsManager.GetColorMapPtr(m_mixerMap2Id);
  m_colorMapsManager.ChangeAllColorMapsNow();

  m_colorMapChangeCompleted =
      m_goomRand.GetRandInRange(MIN_COLOR_MAP_CHANGE_COMPLETED, MAX_COLOR_MAP_CHANGE_COMPLETED);
  m_tAwayFromBaseColor =
      m_goomRand.GetRandInRange(MIN_T_AWAY_FROM_BASE_COLOR, MAX_T_AWAY_FROM_BASE_COLOR);
  m_countSinceColorMapChange = m_colorMapChangeCompleted;
}

auto Colorizer::GetMixedColor(const Pixel& baseColor,
                              const uint32_t hitCount,
                              const float brightness,
                              const float tMix,
                              const float tX,
                              const float tY) const -> Pixel
{
  const float logAlpha =
      m_maxHitCount <= 1 ? 1.0F : (std::log(static_cast<float>(hitCount)) / m_logMaxHitCount);

  Pixel mixColor{};
  float tBaseMix = 0.0F;

  switch (m_colorMode)
  {
    case IfsDancersFx::ColorMode::MAP_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE:
      mixColor = GetNextMixerMapColor(brightness * logAlpha, tX, tY);
      tBaseMix = GetMapColorsTBaseMix();
      break;

    case IfsDancersFx::ColorMode::MIX_COLORS:
    case IfsDancersFx::ColorMode::REVERSE_MIX_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE:
      mixColor = GetNextMixerMapColor(tMix, tX, tY);
      tBaseMix = 1.0F - m_tAwayFromBaseColor;
      break;

    case IfsDancersFx::ColorMode::SINGLE_COLORS:
      mixColor = baseColor;
      tBaseMix = 1.0F - m_tAwayFromBaseColor;
      break;

    case IfsDancersFx::ColorMode::SINE_MIX_COLORS:
    case IfsDancersFx::ColorMode::SINE_MAP_COLORS:
      mixColor = GetSineMixColor(tX, tY);
      tBaseMix = 1.0F - m_tAwayFromBaseColor;
      break;

    default:
      throw std::logic_error("Unknown ColorMode");
  }

  mixColor = GetFinalMixedColor(baseColor, mixColor, tBaseMix);

  return m_colorAdjust.GetAdjustment(brightness * logAlpha, mixColor);
}

auto Colorizer::GetNextMixerMapColor(const float t, const float tX, const float tY) const -> Pixel
{
  const Pixel nextColor =
      IColorMap::GetColorMix(m_colorMapsManager.GetColorMap(m_mixerMap1Id).GetColor(tX),
                             m_colorMapsManager.GetColorMap(m_mixerMap2Id).GetColor(tY), t);
  if (0 == m_countSinceColorMapChange)
  {
    return nextColor;
  }

  const float tTransition = static_cast<float>(m_countSinceColorMapChange) /
                            static_cast<float>(m_colorMapChangeCompleted);
  --m_countSinceColorMapChange;
  const Pixel prevNextColor =
      IColorMap::GetColorMix(m_prevMixerMap1->GetColor(tX), m_prevMixerMap2->GetColor(tY), t);
  return IColorMap::GetColorMix(nextColor, prevNextColor, tTransition);
}

inline auto Colorizer::GetMapColorsTBaseMix() const -> float
{
  if (m_colorMode == IfsDancersFx::ColorMode::MAP_COLORS)
  {
    return 1.0F - m_tAwayFromBaseColor;
  }

  static constexpr float MIN_T_BASE_MIX = 0.3F;
  static constexpr float MAX_T_BASE_MIX = 0.5F;
  return m_goomRand.GetRandInRange(MIN_T_BASE_MIX, MAX_T_BASE_MIX);
}

inline auto Colorizer::GetSineMixColor(const float tX, const float tY) const -> Pixel
{
  static constexpr float INITIAL_FREQ = 20.0F;
  static constexpr float T_MIX_FACTOR = 0.5F;
  static constexpr float Z_STEP = 0.1F;
  static const float s_FREQ = INITIAL_FREQ;
  static float s_z = 0.0F;

  const Pixel mixColor =
      GetNextMixerMapColor(T_MIX_FACTOR * (1.0F + std::sin(s_FREQ * s_z)), tX, tY);

  s_z += Z_STEP;

  return mixColor;
}

inline auto Colorizer::GetFinalMixedColor(const Pixel& baseColor,
                                          const Pixel& mixColor,
                                          const float tBaseMix) const -> Pixel
{
  if (m_colorMode == IfsDancersFx::ColorMode::REVERSE_MIX_COLORS)
  {
    return IColorMap::GetColorMix(mixColor, baseColor, tBaseMix);
  }

  return IColorMap::GetColorMix(baseColor, mixColor, tBaseMix);
}

} // namespace GOOM::VISUAL_FX::IFS
