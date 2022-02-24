#include "colormaps_grids.h"
#include "utils/t_values.h"

#undef NDEBUG
#include <cassert>
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
  assert(m_width == static_cast<uint32_t>(m_verticalColorMaps.size()));
}

auto ColorMapsGrid::GetNextColors() const -> ColorArray
{
  ColorArray nextColors(m_width);

  TValue horizontalT{TValue::StepType::SINGLE_CYCLE, m_width};
  for (size_t i = 0; i < m_width; ++i)
  {
    const Pixel horizontalColor = m_horizontalColorMap->GetColor(horizontalT());
    const Pixel verticalColor = m_verticalColorMaps.at(i)->GetColor(m_verticalT());
    nextColors.at(i) = IColorMap::GetColorMix(horizontalColor, verticalColor, m_colorMixingT(i));

    horizontalT.Increment();
  }

  return nextColors;
}

} // namespace GOOM::COLOR
