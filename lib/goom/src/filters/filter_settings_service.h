#ifndef VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H
#define VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H

#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_settings.h"
#include "filter_speed_coefficients_effect.h"
#include "filter_zoom_vector.h"
#include "goomutils/goomrand.h"
#include "goomutils/mathutils.h"
#include "goomutils/spimpl.h"

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
                        const std::shared_ptr<const GOOM::PluginInfo>& goomInfo,
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
  [[nodiscard]] auto GetROVitesseSetting() const -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesseSetting() -> Vitesse&;

  void ChangeMilieu();
  void SetMiddlePoints();
  void SetNoisifySetting(bool value);
  void SetNoiseFactorSetting(float value);
  void SetBlockyWavySetting(bool value);
  void SetRotateSetting(float value);
  void MultiplyRotateSetting(float factor);
  void ToggleRotateSetting();
  void SetClippedColor(const Pixel& color);

  void SetRandomFilterSettings();

  void SetTranLerpIncrement(int32_t value);
  void SetDefaultTranLerpIncrement();
  void MultiplyTranLerpIncrement(int32_t factor);

  void SetTranLerpToMaxSwitchMult(float value);
  void SetDefaultTranLerpToMaxSwitchMult();

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

  void SetRandomFilterSettings(ZoomFilterMode mode);
  void SetDefaultFilterSettings(ZoomFilterMode mode);

  static const UTILS::Weights<ZoomFilterMode> WEIGHTED_FILTER_EVENTS;
  UTILS::Parallel& m_parallel;
  const std::shared_ptr<const PluginInfo> m_goomInfo;
  const V2dInt m_midScreenPoint;
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

  static constexpr uint32_t DEFAULT_MIDDLE_X = 16;
  static constexpr uint32_t DEFAULT_MIDDLE_Y = 1;
  static constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
  static constexpr float MIN_ROTATE_SPEED = -0.5F;
  static constexpr float MAX_ROTATE_SPEED = +0.5F;
  static constexpr int DEFAULT_TRAN_LERP_INCREMENT = 0x7f;
  static constexpr float DEFAULT_SWITCH_MULT = 29.0F / 30.0F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = 2.01F;
  static constexpr float MAX_MAX_SPEED_COEFF = 4.01F;
  ZoomFilterSettings m_filterSettings;

  bool m_filterEffectsSettingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<ISpeedCoefficientsEffect>&;

  void SetDefaultSettings();
  void SetFilterModeSettings();
  void SetWaveModeSettings();

  void SetRandomMiddlePoints();
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

inline auto FilterSettingsService::GetROVitesseSetting() const -> const Vitesse&
{
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::GetRWVitesseSetting() -> Vitesse&
{
  m_filterEffectsSettingsHaveChanged = true;
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline void FilterSettingsService::ChangeMilieu()
{
  m_filterEffectsSettingsHaveChanged = true;
  SetMaxSpeedCoeff();
  SetMiddlePoints();
}

inline void FilterSettingsService::SetMiddlePoints()
{
  m_filterEffectsSettingsHaveChanged = true;
  SetRandomMiddlePoints();
}

inline void FilterSettingsService::SetRandomFilterSettings()
{
  m_filterEffectsSettingsHaveChanged = true;
  SetRandomFilterSettings(GetNewRandomMode());
}

inline void FilterSettingsService::SetDefaultFilterSettings(const ZoomFilterMode mode)
{
  m_filterEffectsSettingsHaveChanged = true;
  m_previousFilterMode = m_filterMode;
  m_filterMode = mode;
  SetDefaultSettings();
}

inline void FilterSettingsService::SetNoisifySetting(const bool value)
{
  if (m_filterSettings.filterEffectsSettings.noisify == value)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.noisify = value;
}

inline void FilterSettingsService::SetNoiseFactorSetting(const float value)
{
  if (UTILS::floats_equal(m_filterSettings.filterEffectsSettings.noiseFactor, value))
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.noiseFactor = value;
}

inline void FilterSettingsService::SetBlockyWavySetting(const bool value)
{
  m_filterSettings.filterColorSettings.blockyWavy = value;
}

inline void FilterSettingsService::SetClippedColor(const Pixel& color)
{
  m_filterSettings.filterColorSettings.clippedColor = color;
}

inline void FilterSettingsService::SetRotateSetting(const float value)
{
  if (UTILS::floats_equal(m_filterSettings.filterEffectsSettings.rotateSpeed, value))
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotateSpeed = value;
}

inline void FilterSettingsService::MultiplyRotateSetting(const float factor)
{
  if (UTILS::floats_equal(m_filterSettings.filterEffectsSettings.rotateSpeed, 1.0F))
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotateSpeed *= factor;
}

inline void FilterSettingsService::ToggleRotateSetting()
{
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotateSpeed =
      -m_filterSettings.filterEffectsSettings.rotateSpeed;
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

inline void FilterSettingsService::SetDefaultTranLerpToMaxSwitchMult()
{
  SetTranLerpToMaxSwitchMult(DEFAULT_SWITCH_MULT);
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H
