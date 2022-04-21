#include "random_colormaps_manager.h"

#include "colormaps.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::COLOR
{

auto RandomColorMapsManager::AddColorMapInfo(const RandomColorMapsManager::ColorMapInfo& info)
    -> uint32_t
{
  m_infoList.emplace_back(info);
  m_colorMaps.emplace_back(info.colorMaps->GetRandomColorMapPtr(info.types));

  return static_cast<uint32_t>(m_infoList.size() - 1);
}

auto RandomColorMapsManager::RemoveColorMapInfo(const uint32_t id) -> void
{
  m_infoList.erase(begin(m_infoList) + id);
  m_colorMaps.erase(begin(m_colorMaps) + id);
}

auto RandomColorMapsManager::GetColorMap(const size_t id) const -> const IColorMap&
{
  return *m_colorMaps.at(id);
}

auto RandomColorMapsManager::GetColorMapPtr(const size_t id) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(m_colorMaps.at(id));
}

auto RandomColorMapsManager::UpdateColorMapName(const size_t id,
                                                const COLOR_DATA::ColorMapName colorMapName) -> void
{
  m_infoList.at(id).colorMapName = colorMapName;
}

auto RandomColorMapsManager::ChangeAllColorMapsNow() -> void
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    ChangeColorMapNow(id);
  }
}

auto RandomColorMapsManager::ChangeColorMapNow(const size_t id) -> void
{
  const ColorMapInfo& info = m_infoList[id];
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
