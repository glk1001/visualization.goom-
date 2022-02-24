#pragma once

#include "colormaps.h"
#include "goom_graphic.h"
#include "utils/t_values.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM::COLOR
{

class ColorMapsGrid
{
public:
  using ColorMapsArray = std::vector<const IColorMap*>;
  using ColorArray = std::vector<Pixel>;
  using ColorMixingTFunc = std::function<float(size_t colorIndex)>;

  ColorMapsGrid(uint32_t width,
                const IColorMap& horizontalColorMap,
                const UTILS::TValue& verticalT,
                const ColorMapsArray& verticalColorMaps,
                const ColorMixingTFunc& colorMixingTFunc) noexcept;

  void SetColorMaps(const IColorMap& horizontalColorMap, const ColorMapsArray& verticalColorMaps);

  [[nodiscard]] auto GetNextColors() const -> ColorArray;

private:
  const uint32_t m_width;
  const IColorMap* m_horizontalColorMap;
  ColorMapsArray m_verticalColorMaps;
  const ColorMixingTFunc m_colorMixingT;
  const UTILS::TValue& m_verticalT;
};

inline void ColorMapsGrid::SetColorMaps(const IColorMap& horizontalColorMap,
                                        const ColorMapsArray& verticalColorMaps)
{
  m_horizontalColorMap = &horizontalColorMap;
  m_verticalColorMaps = verticalColorMaps;
}

} // namespace GOOM::COLOR
