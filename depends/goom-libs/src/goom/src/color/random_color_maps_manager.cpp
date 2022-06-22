#include "random_color_maps_manager.h"

//#undef NO_LOGGING

#include "color_data/color_map_enums.h"
#include "goom_config.h"
#include "logging.h"

#include <cstdint>
#include <memory>

namespace GOOM::COLOR
{

using UTILS::Logging;

RandomColorMapsManager::ColorMapInfo::~ColorMapInfo() noexcept = default;

auto RandomColorMapsManager::AddDefaultColorMapInfo(const UTILS::MATH::IGoomRand& goomRand) noexcept
    -> ColorMapId
{
  return AddColorMapInfo(
      {MakeSharedAllMapsUnweighted(goomRand), RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

auto RandomColorMapsManager::AddColorMapInfo(
    const RandomColorMapsManager::ColorMapInfo& info) noexcept -> ColorMapId
{
  m_infoList.emplace_back(info);
  m_colorMapPtrs.emplace_back(info.colorMaps->GetRandomColorMapPtr(info.colorMapTypes));

  return ColorMapId{static_cast<int32_t>(m_infoList.size() - 1)};
}

auto RandomColorMapsManager::UpdateColorMapInfo(const ColorMapId id,
                                                const ColorMapInfo& info) noexcept -> void
{
  m_infoList.at(id()) = info;
  m_colorMapPtrs.at(id()) = info.colorMaps->GetRandomColorMapPtr(info.colorMapTypes);
}

auto RandomColorMapsManager::RandomizeColorMaps(const size_t id) noexcept -> void
{
  const ColorMapInfo& info = m_infoList.at(id);
  m_colorMapPtrs.at(id) = info.colorMaps->GetRandomColorMapPtr(info.colorMapTypes);
}

} // namespace GOOM::COLOR
