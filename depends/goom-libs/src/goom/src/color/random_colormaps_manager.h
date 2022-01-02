#pragma once

#include "colormaps.h"
#include "random_colormaps.h"

#include <memory>
#include <set>

namespace GOOM::COLOR
{

class RandomColorMapsManager
{
public:
  RandomColorMapsManager() noexcept = default;

  struct ColorMapInfo
  {
    std::shared_ptr<const RandomColorMaps> colorMaps{};
    COLOR_DATA::ColorMapName colorMapName{COLOR_DATA::ColorMapName::_NULL};
    std::set<RandomColorMaps::ColorMapTypes> types{};
  };

  [[nodiscard]] auto AddColorMapInfo(const ColorMapInfo& info) -> uint32_t;

  void UpdateColorMapName(size_t id, COLOR_DATA::ColorMapName colorMapName);

  void ChangeAllColorMapsNow();
  void ChangeColorMapNow(size_t id);

  [[nodiscard]] auto GetColorMap(size_t id) const -> const IColorMap&;
  [[nodiscard]] auto GetColorMapPtr(size_t id) const -> std::shared_ptr<const IColorMap>;

private:
  std::vector<ColorMapInfo> m_infoList{};
  std::vector<std::shared_ptr<const IColorMap>> m_colorMaps{};

  void DoUpdateColorMapName(size_t id, COLOR_DATA::ColorMapName colorMapName);
  void DoChangeColorMap(size_t id);
};

} // namespace GOOM::COLOR
