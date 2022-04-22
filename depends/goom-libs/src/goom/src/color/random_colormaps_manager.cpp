#include "random_colormaps_manager.h"

#include "colormaps.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::COLOR
{

auto RandomColorMapsManager::AddColorMapInfo(
    const RandomColorMapsManager::ColorMapInfo& info) noexcept -> ColorMapId
{
  m_infoList.emplace_back(info);
  m_colorMaps.emplace_back(info.colorMaps->GetRandomColorMapPtr(info.types));

  return ColorMapId{static_cast<int32_t>(m_infoList.size() - 1)};
}

auto RandomColorMapsManager::RemoveColorMapInfo(const ColorMapId id) noexcept -> void
{
  if (not id.IsSet())
  {
    return;
  }

  m_infoList.erase(begin(m_infoList) + static_cast<std::ptrdiff_t>(id()));
  m_colorMaps.erase(begin(m_colorMaps) + static_cast<std::ptrdiff_t>(id()));
}

auto RandomColorMapsManager::GetColorMap(const ColorMapId id) const noexcept -> const IColorMap&
{
  return *m_colorMaps.at(id());
}

auto RandomColorMapsManager::GetColorMapPtr(const ColorMapId id) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(m_colorMaps.at(id()));
}

auto RandomColorMapsManager::UpdateColorMapName(
    const ColorMapId id, const COLOR_DATA::ColorMapName colorMapName) noexcept -> void
{
  m_infoList.at(id()).colorMapName = colorMapName;
}

auto RandomColorMapsManager::ChangeAllColorMapsNow() noexcept -> void
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    ChangeColorMap(id);
  }
}

auto RandomColorMapsManager::ChangeColorMapNow(const ColorMapId id) noexcept -> void
{
  ChangeColorMap(id());
}

auto RandomColorMapsManager::ChangeColorMap(const size_t id) noexcept -> void
{
  const ColorMapInfo& info = m_infoList.at(id);

  if (info.colorMapName == COLOR_DATA::ColorMapName::_NULL)
  {
    m_colorMaps.at(id) = info.colorMaps->GetRandomColorMapPtr(info.types);
  }
  else
  {
    m_colorMaps.at(id) = info.colorMaps->GetRandomColorMapPtr(info.colorMapName, info.types);
  }
}

} // namespace GOOM::COLOR
