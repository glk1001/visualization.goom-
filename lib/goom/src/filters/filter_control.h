#ifndef VISUALIZATION_GOOM_FILTER_CONTROL_H
#define VISUALIZATION_GOOM_FILTER_CONTROL_H

#include "filter_data.h"
#include "goomutils/spimpl.h"

#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

class PluginInfo;

namespace FILTERS
{

class ImageDisplacement;

class FilterControl
{
public:
  explicit FilterControl(const std::shared_ptr<const GOOM::PluginInfo>& goomInfo) noexcept;

  [[nodiscard]] auto GetResourcesDirectory() const -> const std::string&;
  void SetResourcesDirectory(const std::string& dirName);

  void Start();

  auto HaveSettingsChangedSinceMark() const -> bool;
  void ClearUnchangedMark();

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterData&;
  [[nodiscard]] auto GetVitesseSetting() const -> const Vitesse&;
  [[nodiscard]] auto GetVitesseSetting() -> Vitesse&;

  void SetNoisifySetting(bool value);
  void SetNoiseFactorSetting(float value);
  void SetBlockyWavySetting(bool value);
  void SetRotateSetting(float value);
  void MultiplyRotateSetting(float factor);
  void ToggleRotateSetting();

  void ChangeMilieu();

  void SetRandomFilterSettings();
  void SetRandomFilterSettings(ZoomFilterMode mode);
  void SetDefaultFilterSettings(ZoomFilterMode mode);

private:
  static const UTILS::Weights<ZoomFilterMode> WEIGHTED_FILTER_EVENTS;
  const std::shared_ptr<const PluginInfo> m_goomInfo;
  const V2dInt m_midScreenPoint;
  std::string m_resourcesDirectory{};
  ZoomFilterData m_filterData{};
  class FilterEvents;
  spimpl::unique_impl_ptr<FilterEvents> m_filterEvents;

  static const std::vector<std::string> IMAGE_FILENAMES;
  [[nodiscard]] auto GetImageFilename(const std::string& imageFilename) const -> std::string;
  std::vector<std::shared_ptr<ImageDisplacement>> m_imageDisplacements{};

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;

  bool m_settingsHaveChanged = false;

  void SetDefaultSettings();
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
  void SetWaveModeSettings();
  void SetYOnlyModeSettings();

  void SetMiddlePoints();
  void SetPlaneEffects();
  void SetRotate(float probability);
};

inline auto FilterControl::GetResourcesDirectory() const -> const std::string&
{
  return m_resourcesDirectory;
}

inline void FilterControl::SetResourcesDirectory(const std::string& dirName)
{
  m_resourcesDirectory = dirName;
}

inline auto FilterControl::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_filterData;
}

inline auto FilterControl::HaveSettingsChangedSinceMark() const -> bool
{
  return m_settingsHaveChanged;
}

inline void FilterControl::ClearUnchangedMark()
{
  m_settingsHaveChanged = false;
}

inline auto FilterControl::GetVitesseSetting() const -> const Vitesse&
{
  return m_filterData.vitesse;
}

inline auto FilterControl::GetVitesseSetting() -> Vitesse&
{
  m_settingsHaveChanged = true;
  return m_filterData.vitesse;
}

inline void FilterControl::SetNoisifySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterData.noisify = value;
}

inline void FilterControl::SetNoiseFactorSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterData.noiseFactor = value;
}

inline void FilterControl::SetBlockyWavySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterData.blockyWavy = value;
}

inline void FilterControl::SetRotateSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed = value;
}

inline void FilterControl::MultiplyRotateSetting(const float factor)
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed *= factor;
}

inline void FilterControl::ToggleRotateSetting()
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed = -m_filterData.rotateSpeed;
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_CONTROL_H
