#include "colorizer.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"

#include <cmath>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace IFS
{
#else
namespace GOOM::IFS
{
#endif

using COLOR::GetBrighterColor;
using COLOR::GetSlightlyDivergingStandardMaps;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::IGoomRand;
using UTILS::Weights;
using VISUAL_FX::IfsDancersFx;

Colorizer::Colorizer(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    // clang-format off
    m_colorModeWeights{
        m_goomRand,
        {
            { IfsDancersFx::ColorMode::MAP_COLORS,            15 },
            { IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE, 20 },
            { IfsDancersFx::ColorMode::MIX_COLORS,            15 },
            { IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE, 20 },
            { IfsDancersFx::ColorMode::REVERSE_MIX_COLORS,    15 },
            { IfsDancersFx::ColorMode::SINGLE_COLORS,         10 },
            { IfsDancersFx::ColorMode::SINE_MIX_COLORS,       10 },
            { IfsDancersFx::ColorMode::SINE_MAP_COLORS,       10 },
        }
    }
// clang-format on
{
  SetWeightedColorMaps(GetSlightlyDivergingStandardMaps(m_goomRand));
}

void Colorizer::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;

  m_mixerMap1Id = m_colorMapsManager.AddColorMapInfo(
      {m_colorMaps, ColorMapName::_NULL, RandomColorMaps::EMPTY});
  m_prevMixerMap1 = m_colorMapsManager.GetColorMapPtr(m_mixerMap1Id);
  m_mixerMap2Id = m_colorMapsManager.AddColorMapInfo(
      {m_colorMaps, ColorMapName::_NULL, RandomColorMaps::EMPTY});
  m_prevMixerMap2 = m_colorMapsManager.GetColorMapPtr(m_mixerMap2Id);
}

void Colorizer::ChangeColorMode()
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

void Colorizer::ChangeColorMaps()
{
  m_prevMixerMap1 = m_colorMapsManager.GetColorMapPtr(m_mixerMap1Id);
  m_prevMixerMap2 = m_colorMapsManager.GetColorMapPtr(m_mixerMap2Id);
  m_colorMapsManager.ChangeAllColorMapsNow();

  //  logInfo("prevMixerMap = {}", enumToString(prevMixerMap->GetMapName()));
  //  logInfo("mixerMap = {}", enumToString(mixerMap->GetMapName()));
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

  Pixel mixColor;
  float tBaseMix{};

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

  mixColor = GetFinalMixedColor(mixColor, baseColor, tBaseMix);

  return GetGammaCorrection(brightness * logAlpha, mixColor);
}

inline auto Colorizer::GetNextMixerMapColor(const float t, const float tX, const float tY) const
    -> Pixel
{
  //  const float angle = y == 0.0F ? m_half_pi : std::atan2(y, x);
  //  const Pixel nextColor = mixerMap1->GetColor((m_pi + angle) / m_two_pi);
  const Pixel nextColor =
      IColorMap::GetColorMix(m_colorMapsManager.GetColorMap(m_mixerMap1Id).GetColor(tX),
                             m_colorMapsManager.GetColorMap(m_mixerMap2Id).GetColor(tY), t);
  //  const Pixel nextColor = m_colorMapsManager.GetColorMap(m_mixerMap1Id).GetColor(x);
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

  constexpr float MIN_T_BASE_MIX = 0.3F;
  constexpr float MAX_T_BASE_MIX = 0.5F;
  return m_goomRand.GetRandInRange(MIN_T_BASE_MIX, MAX_T_BASE_MIX);
}

inline auto Colorizer::GetSineMixColor(const float tX, const float tY) const -> Pixel
{
  constexpr float INITIAL_FREQ = 20.0F;
  constexpr float T_MIX_FACTOR = 0.5F;
  constexpr float Z_STEP = 0.1F;
  static const float s_freq = INITIAL_FREQ;
  static float s_z = 0.0F;

  const Pixel mixColor =
      GetNextMixerMapColor(T_MIX_FACTOR * (1.0F + std::sin(s_freq * s_z)), tX, tY);

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

inline auto Colorizer::GetGammaCorrection(const float brightness, const Pixel& color) const -> Pixel
{
  // if constexpr (GAMMA == 1.0F)
  if (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color, true);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace GOOM
#else
} // namespace GOOM::IFS
#endif
