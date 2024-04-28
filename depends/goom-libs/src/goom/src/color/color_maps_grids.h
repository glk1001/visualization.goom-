#pragma once

#include "color_maps.h"
#include "goom/goom_graphic.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

import Goom.Utils;

namespace GOOM::COLOR
{

class ColorMapsGrid
{
public:
  using ColorMixingTFunc = std::function<float(float tX, float tY)>;

  ColorMapsGrid(const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
                const UTILS::TValue& verticalT,
                const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps,
                const ColorMixingTFunc& colorMixingTFunc) noexcept;

  auto SetVerticalT(const UTILS::TValue& val) noexcept -> void;
  auto SetColorMaps(const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
                    const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps) noexcept
      -> void;

  [[nodiscard]] auto GetCurrentHorizontalLineColors() const -> std::vector<Pixel>;

private:
  std::vector<COLOR::ColorMapPtrWrapper> m_horizontalColorMaps;
  float m_maxHorizontalLineIndex = static_cast<float>(m_horizontalColorMaps.size() - 1);
  std::vector<COLOR::ColorMapPtrWrapper> m_verticalColorMaps;
  uint32_t m_width = static_cast<uint32_t>(m_verticalColorMaps.size());

  const UTILS::TValue* m_verticalT;
  ColorMixingTFunc m_colorMixingT;

  [[nodiscard]] auto GetCurrentHorizontalLineIndex() const -> size_t;
};

inline auto ColorMapsGrid::SetVerticalT(const UTILS::TValue& val) noexcept -> void
{
  m_verticalT = &val;
}

inline auto ColorMapsGrid::SetColorMaps(
    const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
    const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps) noexcept -> void
{
  m_horizontalColorMaps    = horizontalColorMaps;
  m_maxHorizontalLineIndex = static_cast<float>(m_horizontalColorMaps.size() - 1);

  m_verticalColorMaps = verticalColorMaps;
  m_width             = static_cast<uint32_t>(m_verticalColorMaps.size());
}

} // namespace GOOM::COLOR
