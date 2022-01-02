#include "random_colormaps_manager.h"

#include "colormaps.h"

#include <format>
#include <stdexcept>


namespace GOOM::COLOR
{

auto RandomColorMapsManager::AddColorMapInfo(const RandomColorMapsManager::ColorMapInfo& info)
    -> uint32_t
{
  m_infoList.emplace_back(info);
  m_colorMaps.emplace_back(info.colorMaps->GetRandomColorMapPtr(info.types));

  return static_cast<uint32_t>(m_infoList.size() - 1);
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

void RandomColorMapsManager::UpdateColorMapName(const size_t id,
                                                const COLOR_DATA::ColorMapName colorMapName)
{
  if (id >= m_infoList.size())
  {
    throw std::logic_error(std20::format("UpdateColorMapName: Invalid color map id: {}.", id));
  }

  DoUpdateColorMapName(id, colorMapName);
}

void RandomColorMapsManager::ChangeColorMapNow(const size_t id)
{
  if (id >= m_infoList.size())
  {
    throw std::logic_error(std20::format("GetColorMapInfo: Invalid color map id: {}.", id));
  }

  DoChangeColorMap(id);
}

void RandomColorMapsManager::ChangeAllColorMapsNow()
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    DoChangeColorMap(id);
  }
}

inline void RandomColorMapsManager::DoUpdateColorMapName(
    const size_t id, const COLOR_DATA::ColorMapName colorMapName)
{
  m_infoList[id].colorMapName = colorMapName;
}

inline void RandomColorMapsManager::DoChangeColorMap(const size_t id)
{
  const ColorMapInfo& info = m_infoList[id];
  if (info.colorMapName == COLOR_DATA::ColorMapName::_NULL)
  {
    m_colorMaps[id] = info.colorMaps->GetRandomColorMapPtr(info.types);
  }
  else
  {
    m_colorMaps[id] = info.colorMaps->GetRandomColorMapPtr(info.colorMapName, info.types);
  }
}

} // namespace GOOM::COLOR
