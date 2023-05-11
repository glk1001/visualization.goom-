//#undef NO_LOGGING

#include "random_color_maps_manager.h"

#include "goom_config.h"
#include "goom_logger.h"
#include "random_color_maps_groups.h"

namespace GOOM::COLOR
{

using UTILS::MATH::IGoomRand;

auto RandomColorMapsManager::AddDefaultColorMapInfo(const IGoomRand& goomRand,
                                                    const PixelChannelType defaultAlpha) noexcept
    -> ColorMapId
{
  const auto weightedColorMaps = WeightedRandomColorMaps{
      RandomColorMapsGroups::MakeSharedAllMapsUnweighted(goomRand), defaultAlpha};

  return AddColorMapInfo({weightedColorMaps, RandomColorMaps::GetAllColorMapsTypes()});
}

auto RandomColorMapsManager::AddColorMapInfo(
    const RandomColorMapsManager::ColorMapInfo& info) noexcept -> ColorMapId
{
  m_infoList.emplace_back(info);
  m_colorMapPtrs.emplace_back(info.colorMaps.GetRandomColorMapSharedPtr(info.colorMapTypes));

  return ColorMapId{static_cast<int32_t>(m_infoList.size() - 1)};
}

auto RandomColorMapsManager::UpdateColorMapInfo(const ColorMapId id,
                                                const ColorMapInfo& info) noexcept -> void
{
  m_infoList.at(id())     = info;
  m_colorMapPtrs.at(id()) = info.colorMaps.GetRandomColorMapSharedPtr(info.colorMapTypes);
}

auto RandomColorMapsManager::RandomizeColorMaps(const size_t id) noexcept -> void
{
  const auto& info      = m_infoList.at(id);
  m_colorMapPtrs.at(id) = info.colorMaps.GetRandomColorMapSharedPtr(info.colorMapTypes);
}

} // namespace GOOM::COLOR
