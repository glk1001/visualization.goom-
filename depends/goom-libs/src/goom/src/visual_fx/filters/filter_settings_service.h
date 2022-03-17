#pragma once

#include "filter_settings.h"
#include "rotation.h"
#include "speed_coefficients_effect.h"
#include "utils/math/goom_rand_base.h"

#include <functional>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

namespace GOOM
{
class Pixel;
class PluginInfo;

namespace VISUAL_FX::FILTERS
{
class ExtraEffectsStates;

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
  _num // unused and must be last
};

class FilterSettingsService
{
public:
  using CreateSpeedCoefficientsEffectFunc = std::function<std::shared_ptr<ISpeedCoefficientsEffect>(
      ZoomFilterMode filterMode,
      const UTILS::MATH::IGoomRand& goomRand,
      const std::string& resourcesDirectory)>;
  // TODO - Visual Studio doesn't like a trailing return type in above function definition.

  FilterSettingsService(const GOOM::PluginInfo& goomInfo,
                        const UTILS::MATH::IGoomRand& goomRand,
                        const std::string& resourcesDirectory,
                        const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect);
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept = delete;
  virtual ~FilterSettingsService() noexcept;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService& = delete;

  void Start();
  void NewCycle();

  void NotifyUpdatedFilterEffectsSettings();
  [[nodiscard]] auto HaveEffectsSettingsChangedSinceLastUpdate() const -> bool;
  [[nodiscard]] auto HasFilterModeChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetCurrentFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetCurrentFilterModeName() const -> const std::string_view&;
  [[nodiscard]] auto GetPreviousFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetPreviousFilterModeName() const -> const std::string_view&;

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterSettings&;
  [[nodiscard]] auto GetROVitesse() const -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesse() -> Vitesse&;

  void ChangeMilieu();
  void ResetRandomExtraEffects();
  void TurnOffRotation();
  void MultiplyRotation(float factor);
  void ToggleRotationDirection();

  void SetNewRandomFilter();

  void SetTranLerpIncrement(int32_t value);
  void SetDefaultTranLerpIncrement();
  void MultiplyTranLerpIncrement(int32_t factor);

  void SetTranLerpToMaxSwitchMult(float value);
  void SetTranLerpToMaxDefaultSwitchMult();

protected:
  void SetFilterMode(ZoomFilterMode filterMode);
  [[nodiscard]] auto GetFilterSettings() -> ZoomFilterSettings&;
  [[nodiscard]] auto GetPluginInfo() const -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const -> const UTILS::MATH::IGoomRand&;
  virtual void SetDefaultSettings();
  virtual void SetRandomZoomMidpoint();
  virtual void SetFilterModeExtraEffects();
  virtual void SetRandomizedExtraEffects();
  virtual void SetWaveModeExtraEffects();
  virtual void UpdateFilterSettingsFromExtraEffects();

private:
  ZoomFilterMode m_filterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_filterModeAtLastUpdate = ZoomFilterMode::NORMAL_MODE;

  void SetRandomSettingsForNewFilterMode();

  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;
  const std::string m_resourcesDirectory;
  std::unique_ptr<ExtraEffectsStates> m_randomizedExtraEffects;

  struct ZoomFilterModeInfo
  {
    const std::string_view name;
    std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect{};
    const float rotateProbability;
    UTILS::MATH::Weights<HypercosOverlay> hypercosWeights;
  };
  std::map<ZoomFilterMode, ZoomFilterModeInfo> m_filterModeData;
  [[nodiscard]] static auto GetFilterModeData(
      const UTILS::MATH::IGoomRand& goomRand,
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
  const UTILS::MATH::ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;

  bool m_filterEffectsSettingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<ISpeedCoefficientsEffect>&;

  enum class ZoomMidpointEvents
  {
    BOTTOM_MID_POINT,
    RIGHT_MID_POINT,
    LEFT_MID_POINT,
    CENTRE_MID_POINT,
    TOP_LEFT_QUARTER_MID_POINT,
    BOTTOM_RIGHT_QUARTER_MID_POINT,
    _num // unused and must be last
  };
  const UTILS::MATH::Weights<ZoomMidpointEvents> m_zoomMidpointWeights;
  [[nodiscard]] auto IsZoomMidpointInTheMiddle() const -> bool;
  void SetAnyRandomZoomMidpoint();
  void SetMaxSpeedCoeff();
};

inline auto FilterSettingsService::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetCurrentFilterMode() const -> ZoomFilterMode
{
  return m_filterMode;
}

inline auto FilterSettingsService::GetCurrentFilterModeName() const -> const std::string_view&
{
  return m_filterModeData.at(m_filterMode).name;
}

inline auto FilterSettingsService::GetPreviousFilterMode() const -> ZoomFilterMode
{
  return m_previousFilterMode;
}

inline auto FilterSettingsService::GetPreviousFilterModeName() const -> const std::string_view&
{
  return m_filterModeData.at(m_previousFilterMode).name;
}

inline auto FilterSettingsService::GetFilterSettings() -> ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetPluginInfo() const -> const PluginInfo&
{
  return m_goomInfo;
}

inline auto FilterSettingsService::GetGoomRand() const -> const UTILS::MATH::IGoomRand&
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
  SetRandomZoomMidpoint();
}

inline void FilterSettingsService::SetFilterMode(const ZoomFilterMode filterMode)
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = filterMode;

  SetRandomSettingsForNewFilterMode();
}

inline void FilterSettingsService::SetNewRandomFilter()
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = GetNewRandomMode();

  SetRandomSettingsForNewFilterMode();
}

inline void FilterSettingsService::TurnOffRotation()
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationEffect = false;
}

inline void FilterSettingsService::MultiplyRotation(const float factor)
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      factor, RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
}

inline void FilterSettingsService::ToggleRotationDirection()
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }

  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationAdjustments.Toggle(
      RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
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
