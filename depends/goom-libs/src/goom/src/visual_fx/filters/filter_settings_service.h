#pragma once

#include "filter_buffers_service.h"
#include "filter_colors_service.h"
#include "filter_settings.h"
#include "rotation.h"
#include "speed_coefficients_effect.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace GOOM
{
class Pixel;
class PluginInfo;

namespace UTILS
{
class IGoomRand;
class Parallel;
} // namespace UTILS

namespace VISUAL_FX::FILTERS
{

enum class ZoomFilterMode
{
  AMULET_MODE = 0,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  DISTANCE_FIELD_MODE,
  HYPERCOS_MODE0,
  HYPERCOS_MODE1,
  HYPERCOS_MODE2,
  HYPERCOS_MODE3,
  IMAGE_DISPLACEMENT_MODE,
  NORMAL_MODE,
  SCRUNCH_MODE,
  SPEEDWAY_MODE0,
  SPEEDWAY_MODE1,
  SPEEDWAY_MODE2,
  WATER_MODE,
  WAVE_MODE0,
  WAVE_MODE1,
  Y_ONLY_MODE,
  _NUM // unused and must be last
};

class FilterSettingsService
{
public:
  using CreateSpeedCoefficientsEffectFunc = std::function<std::shared_ptr<ISpeedCoefficientsEffect>(
      ZoomFilterMode filterMode,
      const UTILS::IGoomRand& goomRand,
      const std::string& resourcesDirectory)>;
  // TODO - Visual Studio doesn't like a trailing return type in above function definition.

  FilterSettingsService(
      UTILS::Parallel& parallel,
      const GOOM::PluginInfo& goomInfo,
      const UTILS::IGoomRand& goomRand,
      const std::string& resourcesDirectory,
      const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect) noexcept;
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept = delete;
  virtual ~FilterSettingsService() noexcept = default;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService& = delete;

  [[nodiscard]] auto GetFilterBuffersService() -> std::unique_ptr<FilterBuffersService>;
  [[nodiscard]] auto GetFilterColorsService() const -> std::unique_ptr<FilterColorsService>;

  void Start();

  void NotifyUpdatedFilterEffectsSettings();
  [[nodiscard]] auto HaveEffectsSettingsChangedSinceLastUpdate() const -> bool;
  [[nodiscard]] auto HasFilterModeChangedSinceLastUpdate() const -> bool;

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

  void SetRandomFilterSettings();

  void SetTranLerpIncrement(int32_t value);
  void SetDefaultTranLerpIncrement();
  void MultiplyTranLerpIncrement(int32_t factor);

  void SetTranLerpToMaxSwitchMult(float value);
  void SetTranLerpToMaxDefaultSwitchMult();

protected:
  void SetFilterMode(ZoomFilterMode filterMode);
  [[nodiscard]] auto GetFilterSettings() -> ZoomFilterSettings&;
  [[nodiscard]] auto GetGoomRand() const -> const UTILS::IGoomRand&;
  [[nodiscard]] virtual auto MakeRotation() const -> std::shared_ptr<Rotation>;
  virtual void SetDefaultSettings();
  virtual void SetRandomExtraEffects();
  virtual void SetFilterModeExtraEffects();
  virtual void SetWaveModeExtraEffects();
  virtual void SetRandomZoomMidPoint();
  virtual void SetRotate(float rotateProbability);

private:
  ZoomFilterMode m_filterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_filterModeAtLastUpdate = ZoomFilterMode::NORMAL_MODE;

  void SetRandomSettingsForNewFilterMode();

  UTILS::Parallel& m_parallel;
  const PluginInfo& m_goomInfo;
  const UTILS::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidPoint;
  const std::string m_resourcesDirectory;
  const NormalizedCoordsConverter m_normalizedCoordsConverter;

  struct ZoomFilterModeInfo
  {
    const std::string name;
    std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect{};
    const float rotateProbability;
    UTILS::Weights<HypercosOverlay> hypercosWeights;
  };
  std::map<ZoomFilterMode, ZoomFilterModeInfo> m_filterModeData;
  [[nodiscard]] static auto GetFilterModeData(
      const UTILS::IGoomRand& goomRand,
      const std::string& resourcesDirectory,
      const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect)
      -> std::map<ZoomFilterMode, ZoomFilterModeInfo>;
  struct FilterModeData
  {
    ZoomFilterMode filterMode;
    const char* name;
    float rotateProb;
    std::vector<std::pair<HypercosOverlay, float>> modeWeights;
  };

  static constexpr uint32_t DEFAULT_ZOOM_MID_X = 16;
  static constexpr uint32_t DEFAULT_ZOOM_MID_Y = 1;
  static constexpr int DEFAULT_TRAN_LERP_INCREMENT = 0x7f;
  static constexpr float DEFAULT_SWITCH_MULT = 29.0F / 30.0F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = 2.01F;
  static constexpr float MAX_MAX_SPEED_COEFF = 4.01F;
  ZoomFilterSettings m_filterSettings;
  const UTILS::ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;

  bool m_filterEffectsSettingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<ISpeedCoefficientsEffect>&;

  enum class ZoomMidPointEvents
  {
    BOTTOM_MID_POINT,
    RIGHT_MID_POINT,
    LEFT_MID_POINT,
    CENTRE_MID_POINT,
    TOP_LEFT_QUARTER_MID_POINT,
    BOTTOM_RIGHT_QUARTER_MID_POINT,
    _NUM // unused and must be last
  };
  const UTILS::Weights<ZoomMidPointEvents> m_zoomMidPointWeights;
  [[nodiscard]] auto IsZoomMidPointInTheMiddle() const -> bool;
  void SetAnyRandomZoomMidPoint();
  void SetMaxSpeedCoeff();
};

inline auto FilterSettingsService::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetFilterSettings() -> ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetGoomRand() const -> const UTILS::IGoomRand&
{
  return m_goomRand;
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

inline void FilterSettingsService::SetFilterMode(const ZoomFilterMode filterMode)
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = filterMode;

  SetRandomSettingsForNewFilterMode();
}

inline void FilterSettingsService::SetRandomFilterSettings()
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = GetNewRandomMode();

  SetRandomSettingsForNewFilterMode();
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

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
