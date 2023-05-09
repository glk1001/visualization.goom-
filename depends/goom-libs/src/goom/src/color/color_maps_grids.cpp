//#undef NO_LOGGING

#include "color_maps_grids.h"

#include "goom_config.h"
#include "goom_logger.h"
#include "utils/t_values.h"

#include <cmath>

namespace GOOM::COLOR
{

using UTILS::TValue;

ColorMapsGrid::ColorMapsGrid(const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
                             const UTILS::TValue& verticalT,
                             const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps,
                             const ColorMixingTFunc& colorMixingTFunc) noexcept
  : m_horizontalColorMaps{horizontalColorMaps},
    m_verticalColorMaps{verticalColorMaps},
    m_verticalT{&verticalT},
    m_colorMixingT{colorMixingTFunc}
{
}

auto ColorMapsGrid::GetCurrentHorizontalLineColors() const -> std::vector<Pixel>
{
  auto nextColors = std::vector<Pixel>(m_width);

  // clang-format off
  const auto& horizontalColorMap = m_horizontalColorMaps.at(GetCurrentHorizontalLineIndex());
  auto horizontalT = TValue{{TValue::StepType::SINGLE_CYCLE, m_width}};
  // clang-format on

  for (auto i = 0U; i < m_width; ++i)
  {
    const auto horizontalColor = horizontalColorMap.GetColor(horizontalT());
    const auto verticalColor   = m_verticalColorMaps.at(i).GetColor((*m_verticalT)());
    const auto mixT            = m_colorMixingT(horizontalT(), (*m_verticalT)());
    nextColors.at(i)           = IColorMap::GetColorMix(horizontalColor, verticalColor, mixT);

    horizontalT.Increment();
  }

  return nextColors;
}

inline auto ColorMapsGrid::GetCurrentHorizontalLineIndex() const -> size_t
{
  Expects(m_maxHorizontalLineIndex >= 0.0F);
  Expects((*m_verticalT)() >= 0.0F);

  return static_cast<size_t>(std::lround((*m_verticalT)() * m_maxHorizontalLineIndex));
}

} // namespace GOOM::COLOR
