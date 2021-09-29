#pragma once

#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_settings.h"
#include "filter_zoom_vector.h"
#include "goom/spimpl.h"
#include "rotation.h"
#include "speed_coefficients_effect.h"
#include "utils/goomrand.h"
#include "utils/mathutils.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

class Pixel;
class PluginInfo;

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace FILTERS
{

class FilterSettingsService
{
public:
  class FilterEvents;

  FilterSettingsService(UTILS::Parallel& parallel,
                        const GOOM::PluginInfo& goomInfo,
                        const std::string& resourcesDirectory) noexcept;

  auto GetFilterBuffersService() -> std::unique_ptr<FilterBuffersService>;
  static auto GetFilterColorsService() -> std::unique_ptr<FilterColorsService>;

  void Start();

  void NotifyUpdatedFilterEffectsSettings();
  auto HaveEffectsSettingsChangedSinceLastUpdate() const -> bool;
  auto HasFilterModeChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetCurrentFilterMode() const -> const std::string&;
  [[nodiscard]] auto GetPreviousFilterMode() const -> const std::string&;

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterSettings&;
  [[nodiscard]] auto GetROVitesse() const -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesse() -> Vitesse&;

  void ChangeMilieu();
  void SetNoise(bool value);
  void SetBlockyWavy(bool value);
  void SetRotateToZero();
  void MultiplyRotate(float factor);
  void ToggleRotate();
  void SetClippedColor(const Pixel& color);

  void SetRandomFilterSettings();

  void SetTranLerpIncrement(int32_t value);
  void SetDefaultTranLerpIncrement();
  void MultiplyTranLerpIncrement(int32_t factor);

  void SetTranLerpToMaxSwitchMult(float value);
  void SetTranLerpToMaxDefaultSwitchMult();

private:
  enum class ZoomFilterMode
  {
    AMULET_MODE = 0,
    CRYSTAL_BALL_MODE0,
    CRYSTAL_BALL_MODE1,
    HYPERCOS_MODE0,
    HYPERCOS_MODE1,
    HYPERCOS_MODE2,
    HYPERCOS_MODE3,
    IMAGE_DISPLACEMENT_MODE,
    NORMAL_MODE,
    SCRUNCH_MODE,
    SPEEDWAY_MODE0,
    SPEEDWAY_MODE1,
    WATER_MODE,
    WAVE_MODE0,
    WAVE_MODE1,
    Y_ONLY_MODE,
    _NUM // unused and must be last
  };

  ZoomFilterMode m_filterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_filterModeAtLastUpdate = ZoomFilterMode::NORMAL_MODE;

  void SetRandomSettingsForNewFilterMode();
  void SetDefaultFilterSettings(ZoomFilterMode mode);

  static const UTILS::Weights<ZoomFilterMode> WEIGHTED_FILTER_EVENTS;
  UTILS::Parallel& m_parallel;
  const PluginInfo& m_goomInfo;
  const V2dInt m_screenMidPoint;
  const std::string m_resourcesDirectory;
  spimpl::unique_impl_ptr<FilterEvents> m_filterEvents;

  struct ZoomFilterModeInfo
  {
    const std::string name;
    std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect{};
    const float rotateProbability;
    UTILS::Weights<HypercosOverlay> hypercosWeights;
  };
  std::map<ZoomFilterMode, ZoomFilterModeInfo> m_filterModeData;
  [[nodiscard]] static auto GetFilterModeData(const std::string& resourcesDirectory)
      -> std::map<ZoomFilterMode, ZoomFilterModeInfo>;

  static constexpr uint32_t DEFAULT_ZOOM_MID_X = 16;
  static constexpr uint32_t DEFAULT_ZOOM_MID_Y = 1;
  static constexpr int DEFAULT_TRAN_LERP_INCREMENT = 0x7f;
  static constexpr float DEFAULT_SWITCH_MULT = 29.0F / 30.0F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = 2.01F;
  static constexpr float MAX_MAX_SPEED_COEFF = 4.01F;
  ZoomFilterSettings m_filterSettings;

  bool m_filterEffectsSettingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<ISpeedCoefficientsEffect>&;
  [[nodiscard]] static auto GetRotation() -> std::shared_ptr<Rotation>;

  void SetDefaultSettings();
  void SetRandomEffects();
  void SetFilterModeSettings();
  void SetWaveModeSettings();

  void SetRandomZoomMidPoint();
  void SetRotate(float rotateProbability);
  void SetMaxSpeedCoeff();
};

inline auto FilterSettingsService::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::HaveEffectsSettingsChangedSinceLastUpdate() const -> bool
{
  return m_filterEffectsSettingsHaveChanged;
}

inline auto FilterSettingsService::HasFilterModeChangedSinceLastUpdate() const -> bool
{
  return m_filterModeAtLastUpdate != m_filterMode;
}

inline void FilterSettingsService::NotifyUpdatedFilterEffectsSettings()
{
  m_filterEffectsSettingsHaveChanged = false;
  m_filterModeAtLastUpdate = m_filterMode;
}

inline auto FilterSettingsService::GetROVitesse() const -> const Vitesse&
{
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::GetRWVitesse() -> Vitesse&
{
  m_filterEffectsSettingsHaveChanged = true;
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline void FilterSettingsService::ChangeMilieu()
{
  m_filterEffectsSettingsHaveChanged = true;
  SetMaxSpeedCoeff();
  SetRandomZoomMidPoint();
}

inline void FilterSettingsService::SetRandomFilterSettings()
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = GetNewRandomMode();

  SetRandomSettingsForNewFilterMode();
}

inline void FilterSettingsService::SetDefaultFilterSettings(const ZoomFilterMode mode)
{
  m_filterEffectsSettingsHaveChanged = true;
  m_previousFilterMode = m_filterMode;
  m_filterMode = mode;
  SetDefaultSettings();
}

inline void FilterSettingsService::SetNoise(const bool value)
{
  if (m_filterSettings.filterEffectsSettings.noiseEffect == value)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.noiseEffect = value;
}

inline void FilterSettingsService::SetBlockyWavy(const bool value)
{
  m_filterSettings.filterColorSettings.blockyWavy = value;
}

inline void FilterSettingsService::SetClippedColor(const Pixel& color)
{
  m_filterSettings.filterColorSettings.clippedColor = color;
}

inline void FilterSettingsService::SetRotateToZero()
{
  if (!m_filterSettings.filterEffectsSettings.rotation->IsActive())
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotation->SetZero();
}

inline void FilterSettingsService::MultiplyRotate(const float factor)
{
  if (!m_filterSettings.filterEffectsSettings.rotation->IsActive())
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotation->Multiply(factor);
}

inline void FilterSettingsService::ToggleRotate()
{
  if (!m_filterSettings.filterEffectsSettings.rotation->IsActive())
  {
    return;
  }

  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotation->Toggle();
}

inline void FilterSettingsService::SetTranLerpIncrement(const int32_t value)
{
  m_filterSettings.filterBufferSettings.tranLerpIncrement = value;
}

inline void FilterSettingsService::SetDefaultTranLerpIncrement()
{
  SetTranLerpIncrement(DEFAULT_TRAN_LERP_INCREMENT);
}

inline void FilterSettingsService::MultiplyTranLerpIncrement(const int32_t factor)
{
  m_filterSettings.filterBufferSettings.tranLerpIncrement *= factor;
}

inline void FilterSettingsService::SetTranLerpToMaxSwitchMult(const float value)
{
  m_filterSettings.filterBufferSettings.tranLerpToMaxSwitchMult = value;
}

inline void FilterSettingsService::SetTranLerpToMaxDefaultSwitchMult()
{
  SetTranLerpToMaxSwitchMult(DEFAULT_SWITCH_MULT);
}

} // namespace FILTERS
} // namespace GOOM

