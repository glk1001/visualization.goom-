#pragma once

#include "colormaps.h"
#include "random_colormaps.h"

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

namespace GOOM::COLOR
{

class RandomColorMapsManager
{
public:
  RandomColorMapsManager() = default;

  struct ColorMapInfo
  {
    std::shared_ptr<const RandomColorMaps> colorMaps{};
    COLOR_DATA::ColorMapName colorMapName{COLOR_DATA::ColorMapName::_NULL};
    std::set<RandomColorMaps::ColorMapTypes> types{};
  };

  [[nodiscard]] auto AddColorMapInfo(const ColorMapInfo& info) -> uint32_t;
  auto RemoveColorMapInfo(uint32_t id) -> void;

  auto UpdateColorMapName(size_t id, COLOR_DATA::ColorMapName colorMapName) -> void;

  auto ChangeAllColorMapsNow() -> void;
  auto ChangeColorMapNow(size_t id) -> void;

  [[nodiscard]] auto GetColorMap(size_t id) const -> const IColorMap&;
  [[nodiscard]] auto GetColorMapPtr(size_t id) const -> std::shared_ptr<const IColorMap>;

private:
  std::vector<ColorMapInfo> m_infoList{};
  std::vector<std::shared_ptr<const IColorMap>> m_colorMaps{};
};

} // namespace GOOM::COLOR
