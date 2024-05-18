module;

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

export module Goom.Color.ColorMapsGrid;

import Goom.Color.ColorMaps;
import Goom.Utils.Math.TValues;
import Goom.Lib.GoomGraphic;

export namespace GOOM::COLOR
{

class ColorMapsGrid
{
public:
  using ColorMixingTFunc = std::function<float(float tX, float tY)>;

  ColorMapsGrid(const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
                const UTILS::MATH::TValue& verticalT,
                const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps,
                const ColorMixingTFunc& colorMixingTFunc) noexcept;

  auto SetVerticalT(const UTILS::MATH::TValue& val) noexcept -> void;
  auto SetColorMaps(const std::vector<COLOR::ColorMapPtrWrapper>& horizontalColorMaps,
                    const std::vector<COLOR::ColorMapPtrWrapper>& verticalColorMaps) noexcept
      -> void;

  [[nodiscard]] auto GetCurrentHorizontalLineColors() const -> std::vector<Pixel>;

private:
  std::vector<COLOR::ColorMapPtrWrapper> m_horizontalColorMaps;
  float m_maxHorizontalLineIndex = static_cast<float>(m_horizontalColorMaps.size() - 1);
  std::vector<COLOR::ColorMapPtrWrapper> m_verticalColorMaps;
  uint32_t m_width = static_cast<uint32_t>(m_verticalColorMaps.size());

  const UTILS::MATH::TValue* m_verticalT;
  ColorMixingTFunc m_colorMixingT;

  [[nodiscard]] auto GetCurrentHorizontalLineIndex() const -> size_t;
};

} // namespace GOOM::COLOR

namespace GOOM::COLOR
{

inline auto ColorMapsGrid::SetVerticalT(const UTILS::MATH::TValue& val) noexcept -> void
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
