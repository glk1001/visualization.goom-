#include "color_maps_grids.h"
#include "goom_config.h"
#include "utils/t_values.h"

#include <cstdint>

namespace GOOM::COLOR
{

using UTILS::TValue;

ColorMapsGrid::ColorMapsGrid(const uint32_t width,
                             const IColorMap& horizontalColorMap,
                             const UTILS::TValue& verticalT,
                             const ColorMapsArray& verticalColorMaps,
                             const ColorMixingTFunc& colorMixingTFunc) noexcept
  : m_width{width},
    m_horizontalColorMap{&horizontalColorMap},
    m_verticalColorMaps{verticalColorMaps},
    m_colorMixingT{colorMixingTFunc},
    m_verticalT{verticalT}
{
  Expects(width == static_cast<uint32_t>(verticalColorMaps.size()));
}

auto ColorMapsGrid::GetNextColors() const -> ColorArray
{
  auto nextColors = ColorArray(m_width);

  auto horizontalT = TValue{TValue::StepType::SINGLE_CYCLE, m_width};
  for (auto i = 0U; i < m_width; ++i)
  {
    const auto horizontalColor = m_horizontalColorMap->GetColor(horizontalT());
    const auto verticalColor = m_verticalColorMaps.at(i)->GetColor(m_verticalT());
    nextColors.at(i) = IColorMap::GetColorMix(horizontalColor, verticalColor, m_colorMixingT(i));

    horizontalT.Increment();
  }

  return nextColors;
}

} // namespace GOOM::COLOR
