#include "filter_buffer_color_info.h"

#include "color/colorutils.h"

#undef NDEBUG

#include <cassert>
#include <format>

namespace GOOM::VISUAL_FX::FILTERS
{

using COLOR::LUMA_BLUE_COMPONENT;
using COLOR::LUMA_GREEN_COMPONENT;
using COLOR::LUMA_RED_COMPONENT;

auto FilterBufferColorInfo::GetRegionInfoArray(
    const std::array<size_t, NUM_Y_REGIONS>& yRegionBorders) noexcept
    -> std::array<RegionInfo, NUM_REGIONS>
{
  std::array<RegionInfo, NUM_REGIONS> regionRectArray{};

  size_t regionIndex = 0;
  size_t y0 = 0;
  for (const auto& yBorder : yRegionBorders)
  {
    const size_t y1 = yBorder;
    assert(y0 <= y1);

    for (size_t xRegionIndex = 0; xRegionIndex < NUM_X_REGIONS; ++xRegionIndex)
    {
      regionRectArray.at(regionIndex) = {y0, y1, xRegionIndex};
      ++regionIndex;
    }

    y0 = y1 + 1;
  }

  assert(regionIndex == NUM_REGIONS);

  return regionRectArray;
}

auto FilterBufferColorInfo::GetFilterBufferRowColorInfoArray(
    const uint32_t height, const std::array<size_t, NUM_X_REGIONS>& xRegionBorders) noexcept
    -> std::vector<FilterBufferRowColorInfo>
{
  std::vector<FilterBufferRowColorInfo> filterBufferRowColorInfo{};
  filterBufferRowColorInfo.reserve(height);

  for (size_t y = 0; y < height; ++y)
  {
    filterBufferRowColorInfo.emplace_back(xRegionBorders);
  }

  return filterBufferRowColorInfo;
}

inline auto FilterBufferColorInfo::FilterBufferRowColorInfo::GetXRegionCounts(
    const size_t xRegionIndex) const noexcept -> Counts
{
  return m_xRegionCountsArray.at(xRegionIndex);
}

inline auto FilterBufferColorInfo::GetAverageLuminance(const Counts& totals) noexcept -> float
{
  if (0 == totals.numNonzeroInRow)
  {
    return 0.0F;
  }

  return ((LUMA_RED_COMPONENT * static_cast<float>(totals.sumRedInRow)) +
          (LUMA_GREEN_COMPONENT * static_cast<float>(totals.sumGreenInRow)) +
          (LUMA_BLUE_COMPONENT * static_cast<float>(totals.sumBlueInRow))) /
         (static_cast<float>(totals.numNonzeroInRow) * channel_limits<float>::max());
}

void FilterBufferColorInfo::CalculateLuminances() noexcept
{
  for (size_t i = 0; i < NUM_REGIONS; ++i)
  {
    m_regionAverageLuminances.at(i) = GetRegionAverageLuminance(i);
  }
}

auto FilterBufferColorInfo::GetRegionAverageLuminance(const size_t regionIndex) const noexcept
    -> float
{
  const RegionInfo& regionInfo = m_regionInfoArray.at(regionIndex);

  Counts totals{};

  for (size_t y = regionInfo.y0; y <= regionInfo.y1; ++y)
  {
    const Counts& xRegionCounts =
        m_filterBufferRowColorInfoArray.at(y).GetXRegionCounts(regionInfo.xRegionIndex);

    totals.numNonzeroInRow += xRegionCounts.numNonzeroInRow;
    totals.sumRedInRow += xRegionCounts.sumRedInRow;
    totals.sumGreenInRow += xRegionCounts.sumGreenInRow;
    totals.sumBlueInRow += xRegionCounts.sumBlueInRow;
  }

  return GetAverageLuminance(totals);
}

auto FilterBufferColorInfo::GetMaxRegionAverageLuminance() const noexcept -> float
{
  float maxAverageLuminance = 0.0F;
  for (const float regionAverageLuminance : m_regionAverageLuminances)
  {
    if (regionAverageLuminance > maxAverageLuminance)
    {
      maxAverageLuminance = regionAverageLuminance;
    }
  }

  return maxAverageLuminance;
}

auto FilterBufferColorInfo::GetRegionAverageLuminanceAtPoint(const Point2dInt& point) const noexcept
    -> float
{
  return m_regionAverageLuminances.at(GetRegionIndexOfPoint(point));
}

inline auto FilterBufferColorInfo::GetRegionIndexOfPoint(const Point2dInt& point) const noexcept
    -> size_t
{
  for (size_t regionIndex = 0; regionIndex < NUM_REGIONS; ++regionIndex)
  {
    const RegionInfo& regionInfo = m_regionInfoArray.at(regionIndex);
    const auto y0 = static_cast<int32_t>(regionInfo.y0);
    const auto y1 = static_cast<int32_t>(regionInfo.y1);

    if ((y0 <= point.y) && (point.y <= y1) && IsInXRegion(point.x, regionInfo.xRegionIndex))
    {
      return regionIndex;
    }
  }

  assert(false);
  return 0;
}

inline auto FilterBufferColorInfo::IsInXRegion(const int32_t x,
                                               const size_t xRegionIndex) const noexcept -> bool
{
  const auto x0 =
      0 == xRegionIndex ? 0 : static_cast<int32_t>(m_xRegionBorders.at(xRegionIndex - 1));
  const auto x1 = static_cast<int32_t>(m_xRegionBorders.at(xRegionIndex));

  return (x0 <= x) && (x <= x1);
}

auto FilterBufferColorInfo::GetAverageLuminanceTest() const noexcept -> float
{
  Counts totals{};

  for (const auto& filterBufferRowColorInfo : m_filterBufferRowColorInfoArray)
  {
    const Counts regionTotals = filterBufferRowColorInfo.GetXRegionTotals();

    totals.numNonzeroInRow += regionTotals.numNonzeroInRow;
    totals.sumRedInRow += regionTotals.sumRedInRow;
    totals.sumGreenInRow += regionTotals.sumGreenInRow;
    totals.sumBlueInRow += regionTotals.sumBlueInRow;
  }

  return GetAverageLuminance(totals);
}

auto FilterBufferColorInfo::FilterBufferRowColorInfo::GetXRegionTotals() const noexcept -> Counts
{
  Counts totals{};
  for (const auto& regionCounts : m_xRegionCountsArray)
  {
    totals.numNonzeroInRow += regionCounts.numNonzeroInRow;
    totals.sumRedInRow += regionCounts.sumRedInRow;
    totals.sumGreenInRow += regionCounts.sumGreenInRow;
    totals.sumBlueInRow += regionCounts.sumBlueInRow;
  }

  return totals;
}

} // namespace GOOM::VISUAL_FX::FILTERS
