#pragma once

#include "color/colorutils.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "utils/math/misc.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::FILTERS
{

class FilterBufferColorInfo
{
public:
  static constexpr uint32_t NUM_X_REGIONS = 5;
  static constexpr uint32_t NUM_Y_REGIONS = 3;

  class FilterBufferRowColorInfo
  {
  public:
    explicit FilterBufferRowColorInfo(
        const std::array<size_t, NUM_X_REGIONS>& xRegionBorders) noexcept;

    void Reset();
    void UpdateColor(const Pixel& color);
    void NextX();

  private:
    const std::array<size_t, NUM_X_REGIONS>& m_xRegionBorders;
    size_t m_currentX = 0;
    size_t m_currentXRegionIndex = 0;

    struct Counts
    {
      uint32_t numNonzeroInRow = 0;
      uint32_t sumRedInRow = 0;
      uint32_t sumGreenInRow = 0;
      uint32_t sumBlueInRow = 0;
    };
    std::array<Counts, NUM_X_REGIONS> m_xRegionCountsArray{};
    [[nodiscard]] auto GetXRegionTotals() const -> Counts;
    [[nodiscard]] auto GetXRegionCounts(size_t xRegionIndex) const -> Counts;

    friend class FilterBufferColorInfo;
  };

  explicit FilterBufferColorInfo(uint32_t width, uint32_t height) noexcept;

  [[nodiscard]] auto GetRow(size_t y) -> FilterBufferRowColorInfo&;
  void CalculateLuminances();
  [[nodiscard]] auto GetMaxRegionAverageLuminance() const -> float;
  [[nodiscard]] auto GetRegionAverageLuminanceAtPoint(const Point2dInt& point) const -> float;

private:
  const uint32_t m_width;
  const uint32_t m_height;

  const std::array<size_t, NUM_X_REGIONS> m_xRegionBorders;
  const std::array<size_t, NUM_Y_REGIONS> m_yRegionBorders;
  template<uint32_t numRegions>
  [[nodiscard]] static auto GetRegionBorders(uint32_t length) -> std::array<size_t, numRegions>;

  std::vector<FilterBufferRowColorInfo> m_filterBufferRowColorInfoArray;
  [[nodiscard]] static auto GetFilterBufferRowColorInfoArray(
      uint32_t height, const std::array<size_t, NUM_X_REGIONS>& xRegionBorders)
      -> std::vector<FilterBufferRowColorInfo>;

  static constexpr uint32_t NUM_REGIONS = NUM_X_REGIONS * NUM_Y_REGIONS;
  struct RegionInfo
  {
    size_t y0;
    size_t y1;
    size_t xRegionIndex;
  };
  const std::array<RegionInfo, NUM_REGIONS> m_regionInfoArray;
  [[nodiscard]] static auto GetRegionInfoArray(
      const std::array<size_t, NUM_Y_REGIONS>& yRegionBorders)
      -> std::array<RegionInfo, NUM_REGIONS>;
  [[nodiscard]] auto GetRegionIndexOfPoint(const Point2dInt& point) const -> size_t;
  [[nodiscard]] auto IsInXRegion(int32_t x, size_t xRegionIndex) const -> bool;

  std::array<float, NUM_REGIONS> m_regionAverageLuminances;
  [[nodiscard]] auto GetRegionAverageLuminance(size_t regionIndex) const -> float;
  using Counts = FilterBufferRowColorInfo::Counts;
  [[nodiscard]] static auto GetAverageLuminance(const Counts& totals) -> float;

  [[nodiscard]] auto GetAverageLuminanceTest() const -> float;
};

inline FilterBufferColorInfo::FilterBufferColorInfo(const uint32_t width,
                                                    const uint32_t height) noexcept
  : m_width{width},
    m_height{height},
    m_xRegionBorders{GetRegionBorders<NUM_X_REGIONS>(m_width)},
    m_yRegionBorders{GetRegionBorders<NUM_Y_REGIONS>(m_height)},
    m_filterBufferRowColorInfoArray{GetFilterBufferRowColorInfoArray(m_height, m_xRegionBorders)},
    m_regionInfoArray{GetRegionInfoArray(m_yRegionBorders)}
{
  static_assert(UTILS::MATH::IsOdd(NUM_X_REGIONS));
  static_assert(UTILS::MATH::IsOdd(NUM_Y_REGIONS));
}

inline auto FilterBufferColorInfo::GetRow(const size_t y) -> FilterBufferRowColorInfo&
{
  return m_filterBufferRowColorInfoArray[y];
}

template<uint32_t numRegions>
auto FilterBufferColorInfo::GetRegionBorders(const uint32_t length)
    -> std::array<size_t, numRegions>
{
  static_assert(numRegions > 0);
  assert(length > 0);
  const uint32_t regionSize = length / numRegions;

  std::array<size_t, numRegions> regionBorders{};
  size_t border = regionSize - 1;
  for (auto& regionBorder : regionBorders)
  {
    regionBorder = border;

    border += regionSize;
    if (border >= length)
    {
      border = length - 1;
    }
    assert(border < length);
  }

  return regionBorders;
}

inline FilterBufferColorInfo::FilterBufferRowColorInfo::FilterBufferRowColorInfo(
    const std::array<size_t, NUM_X_REGIONS>& xRegionBorders) noexcept
  : m_xRegionBorders{xRegionBorders}
{
}

inline void FilterBufferColorInfo::FilterBufferRowColorInfo::Reset()
{
  m_currentX = 0;
  m_currentXRegionIndex = 0;

  for (auto& regionCounts : m_xRegionCountsArray)
  {
    regionCounts.numNonzeroInRow = 0;
    regionCounts.sumRedInRow = 0;
    regionCounts.sumGreenInRow = 0;
    regionCounts.sumBlueInRow = 0;
  }
}

inline void FilterBufferColorInfo::FilterBufferRowColorInfo::UpdateColor(const Pixel& color)
{
  if (COLOR::IsCloseToBlack(color))
  {
    return;
  }

  Counts& regionCounts = m_xRegionCountsArray.at(m_currentXRegionIndex);
  ++regionCounts.numNonzeroInRow;
  regionCounts.sumRedInRow += color.R();
  regionCounts.sumGreenInRow += color.G();
  regionCounts.sumBlueInRow += color.B();
}

inline void FilterBufferColorInfo::FilterBufferRowColorInfo::NextX()
{
  ++m_currentX;
  if (m_currentX > m_xRegionBorders.at(m_currentXRegionIndex))
  {
    ++m_currentXRegionIndex;
  }
}

} // namespace GOOM::VISUAL_FX::FILTERS
