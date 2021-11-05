#pragma once

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom_graphic.h"
#include "utils/goom_rand_base.h"
#include "visual_fx/ifs_dancers_fx.h"

#include <cmath>
#include <cstdint>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace IFS
{
#else
namespace GOOM::IFS
{
#endif

class Colorizer
{
public:
  explicit Colorizer(UTILS::IGoomRand& goomRand) noexcept;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  auto GetColorMaps() const -> const COLOR::RandomColorMaps&;

  auto GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
  void SetForcedColorMode(VISUAL_FX::IfsDancersFx::ColorMode val);
  void ChangeColorMode();

  void ChangeColorMaps();

  void SetMaxHitCount(uint32_t val);

  [[nodiscard]] auto GetMixedColor(const Pixel& baseColor,
                                   uint32_t hitCount,
                                   float brightness,
                                   float tMix,
                                   float tX,
                                   float tY) const -> Pixel;

private:
  UTILS::IGoomRand& m_goomRand;
  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps{};
  COLOR::RandomColorMapsManager m_colorMapsManager{};
  uint32_t m_mixerMap1Id{};
  std::shared_ptr<const COLOR::IColorMap> m_prevMixerMap1{};
  uint32_t m_mixerMap2Id{};
  std::shared_ptr<const COLOR::IColorMap> m_prevMixerMap2{};
  mutable uint32_t m_countSinceColorMapChange = 0;
  static constexpr uint32_t MIN_COLOR_MAP_CHANGE_COMPLETED = 500;
  static constexpr uint32_t MAX_COLOR_MAP_CHANGE_COMPLETED = 1000;
  uint32_t m_colorMapChangeCompleted = MIN_COLOR_MAP_CHANGE_COMPLETED;

  VISUAL_FX::IfsDancersFx::ColorMode m_colorMode = VISUAL_FX::IfsDancersFx::ColorMode::MAP_COLORS;
  VISUAL_FX::IfsDancersFx::ColorMode m_forcedColorMode = VISUAL_FX::IfsDancersFx::ColorMode::_NULL;
  uint32_t m_maxHitCount = 0;
  float m_logMaxHitCount = 0.0;
  static constexpr float MIN_T_AWAY_FROM_BASE_COLOR = 0.0F;
  static constexpr float MAX_T_AWAY_FROM_BASE_COLOR = 0.4F;
  static constexpr float INITIAL_T_AWAY_FROM_BASE_COLOR = 0.0F;
  float m_tAwayFromBaseColor = INITIAL_T_AWAY_FROM_BASE_COLOR; // in [0, 1]
  const UTILS::Weights<VISUAL_FX::IfsDancersFx::ColorMode> m_colorModeWeights;
  auto GetNextColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
  [[nodiscard]] auto GetNextMixerMapColor(float t, float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetSineMixColor(float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetMapColorsTBaseMix() const -> float;
  [[nodiscard]] auto GetFinalMixedColor(const Pixel& baseColor,
                                        const Pixel& mixColor,
                                        float tBaseMix) const -> Pixel;

  static constexpr float GAMMA = 1.5F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  const COLOR::GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

inline auto Colorizer::GetColorMaps() const -> const COLOR::RandomColorMaps&
{
  return *m_colorMaps;
}

inline auto Colorizer::GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode
{
  return m_colorMode;
}

inline void Colorizer::SetForcedColorMode(const VISUAL_FX::IfsDancersFx::ColorMode val)
{
  m_forcedColorMode = val;
}

inline void Colorizer::SetMaxHitCount(const uint32_t val)
{
  m_maxHitCount = val;
  m_logMaxHitCount = std::log(static_cast<float>(m_maxHitCount));
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace GOOM
#else
} // namespace GOOM::IFS
#endif

