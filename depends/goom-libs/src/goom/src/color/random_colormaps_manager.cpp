#include "random_colormaps_manager.h"

#include "colormaps.h"
#include "random_colormaps.h"

#include <format>
#include <stdexcept>


#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
#else
namespace GOOM::COLOR
{
#endif

void RandomColorMapsManager::IncCounter()
{
}

void RandomColorMapsManager::ResetCounter()
{
}

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

auto RandomColorMapsManager::GetColorMapInfo(const size_t id) const
    -> const RandomColorMapsManager::ColorMapInfo&
{
  if (id >= m_infoList.size())
  {
    throw std::logic_error(std20::format("GetColorMapInfo: Invalid color map id: {}.", id));
  }

  return m_infoList[id];
}

void RandomColorMapsManager::UpdateColorMapInfo(const size_t id,
                                                const RandomColorMapsManager::ColorMapInfo& info)
{
  if (id >= m_infoList.size())
  {
    throw std::logic_error(std20::format("UpdateColorMapInfo: Invalid color map id: {}.", id));
  }

  DoUpdateColorMap(id, info);
}

void RandomColorMapsManager::UpdateAllColorMapInfo(const ColorMapInfo& info)
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    DoUpdateColorMap(id, info);
  }
}

void RandomColorMapsManager::UpdateAllColorMapNames(const COLOR_DATA::ColorMapName colorMapName)
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    DoUpdateColorMapName(id, colorMapName);
  }
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

void RandomColorMapsManager::UpdateColorMapTypes(
    const size_t id, const std::set<RandomColorMaps::ColorMapTypes>& types)
{
  if (id >= m_infoList.size())
  {
    throw std::logic_error(std20::format("UpdateColorMapTypes: Invalid color map id: {}.", id));
  }

  DoUpdateColorMapTypes(id, types);
}

void RandomColorMapsManager::UpdateAllColorMapTypes(
    const std::set<RandomColorMaps::ColorMapTypes>& types)
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    DoUpdateColorMapTypes(id, types);
  }
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

inline void RandomColorMapsManager::DoUpdateColorMap(const size_t id, const ColorMapInfo& info)
{
  m_infoList[id] = info;
}

inline void RandomColorMapsManager::DoUpdateColorMapName(
    const size_t id, const COLOR_DATA::ColorMapName colorMapName)
{
  m_infoList[id].colorMapName = colorMapName;
}

inline void RandomColorMapsManager::DoUpdateColorMapTypes(
    const size_t id, const std::set<RandomColorMaps::ColorMapTypes>& types)
{
  m_infoList[id].types = types;
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

#if __cplusplus <= 201402L
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR
#endif