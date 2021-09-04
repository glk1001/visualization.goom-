#ifndef VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H
#define VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H

#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_settings.h"
#include "filter_speed_coefficients_effect.h"
#include "filter_zoom_vector.h"
#include "goomutils/mathutils.h"
#include "goomutils/spimpl.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

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

  void NotifyUpdatedFilterSettings();
  auto HaveSettingsChangedSinceLastUpdate() const -> bool;
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
  void ReduceNoiseFactor();
  void SetBlockyWavySetting(bool value);
  void SetRotateSetting(float value);
  void MultiplyRotateSetting(float factor);
  void ToggleRotateSetting();

  void SetRandomFilterSettings();

  void SetTranLerpIncrement(int32_t value);
  void SetDefaultTranLerpIncrement();
  void MultiplyTranLerpIncrement(int32_t factor);

  void SetTranLerpToMaxSwitchMult(float value);
  void SetDefaultTranLerpToMaxSwitchMult();

private:
  enum class ZoomFilterMode
  {
    _NULL = -1,
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
    SPEEDWAY_MODE,
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
    std::string name{};
    float rotateProbability{};
    std::shared_ptr<SpeedCoefficientsEffect> speedCoefficientsEffect{};
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
  ZoomFilterSettings m_filterSettings;

  bool m_settingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<SpeedCoefficientsEffect>&;

  void SetDefaultSettings();
  void SetFilterModeSettings();
  void SetAmuletModeSettings();
  void SetCrystalBall0ModeSettings();
  void SetCrystalBall1ModeSettings();
  void SetHypercosMode0Settings();
  void SetHypercosMode1Settings();
  void SetHypercosMode2Settings();
  void SetHypercosMode3Settings();
  void SetImageDisplacementModeSettings();
  void SetNormalModeSettings();
  void SetScrunchModeSettings();
  void SetSpeedwayModeSettings();
  void SetWaterModeSettings();
  void SetWaveMode0Settings();
  void SetWaveMode1Settings();
  void SetWaveModeSettings();
  void SetYOnlyModeSettings();

  void SetRandomMiddlePoints();
  void SetRotate(float rotateProbability);
};

inline auto FilterSettingsService::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::HaveSettingsChangedSinceLastUpdate() const -> bool
{
  return m_settingsHaveChanged;
}

inline auto FilterSettingsService::HasFilterModeChangedSinceLastUpdate() const -> bool
{
  return m_filterModeAtLastUpdate != m_filterMode;
}

inline void FilterSettingsService::NotifyUpdatedFilterSettings()
{
  m_settingsHaveChanged = false;
  m_filterModeAtLastUpdate = m_filterMode;
}

inline auto FilterSettingsService::GetROVitesseSetting() const -> const Vitesse&
{
  return m_filterSettings.vitesse;
}

inline auto FilterSettingsService::GetRWVitesseSetting() -> Vitesse&
{
  m_settingsHaveChanged = true;
  return m_filterSettings.vitesse;
}

inline void FilterSettingsService::ChangeMilieu()
{
  m_settingsHaveChanged = true;
  SetMiddlePoints();
}

inline void FilterSettingsService::SetMiddlePoints()
{
  m_settingsHaveChanged = true;
  SetRandomMiddlePoints();
}

inline void FilterSettingsService::SetRandomFilterSettings()
{
  m_settingsHaveChanged = true;
  SetRandomFilterSettings(GetNewRandomMode());
}

inline void FilterSettingsService::SetDefaultFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;
  m_previousFilterMode = m_filterMode;
  m_filterMode = mode;
  SetDefaultSettings();
}

inline void FilterSettingsService::SetNoisifySetting(const bool value)
{
  if (m_filterSettings.noisify == value)
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.noisify = value;
}

inline void FilterSettingsService::SetNoiseFactorSetting(const float value)
{
  if (UTILS::floats_equal(m_filterSettings.noiseFactor, value))
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.noiseFactor = value;
}

inline void FilterSettingsService::ReduceNoiseFactor()
{
  if (!GetFilterSettings().noisify)
  {
    return;
  }
  constexpr float REDUCING_FACTOR = 0.94F;
  const float reducedNoiseFactor = m_filterSettings.noiseFactor * REDUCING_FACTOR;
  SetNoiseFactorSetting(reducedNoiseFactor);
}

inline void FilterSettingsService::SetBlockyWavySetting(const bool value)
{
  if (m_filterSettings.blockyWavy == value)
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.blockyWavy = value;
}

inline void FilterSettingsService::SetRotateSetting(const float value)
{
  if (UTILS::floats_equal(m_filterSettings.rotateSpeed, value))
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed = value;
}

inline void FilterSettingsService::MultiplyRotateSetting(const float factor)
{
  if (UTILS::floats_equal(m_filterSettings.rotateSpeed, 1.0F))
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed *= factor;
}

inline void FilterSettingsService::ToggleRotateSetting()
{
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed = -m_filterSettings.rotateSpeed;
}

inline void FilterSettingsService::SetTranLerpIncrement(const int32_t value)
{
  if (m_filterSettings.tranLerpIncrement == value)
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.tranLerpIncrement = value;
}

inline void FilterSettingsService::SetDefaultTranLerpIncrement()
{
  SetTranLerpIncrement(DEFAULT_TRAN_LERP_INCREMENT);
}

inline void FilterSettingsService::MultiplyTranLerpIncrement(const int32_t factor)
{
  if (1 == factor)
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.tranLerpIncrement *= factor;
}

inline void FilterSettingsService::SetTranLerpToMaxSwitchMult(const float value)
{
  if (UTILS::floats_equal(m_filterSettings.tranLerpToMaxSwitchMult, value))
  {
    return;
  }
  m_settingsHaveChanged = true;
  m_filterSettings.tranLerpToMaxSwitchMult = value;
}

inline void FilterSettingsService::SetDefaultTranLerpToMaxSwitchMult()
{
  SetTranLerpToMaxSwitchMult(DEFAULT_SWITCH_MULT);
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_SETTINGS_SERVICE_H
