#include "filter_buffer_color_info.h"

#include "color/colorutils.h"

#undef NDEBUG

#include <cassert>

namespace GOOM::VISUAL_FX::FILTERS
{

using COLOR::LUMA_BLUE_COMPONENT;
using COLOR::LUMA_GREEN_COMPONENT;
using COLOR::LUMA_RED_COMPONENT;

auto FilterBufferColorInfo::GetXRegionBorders(const uint32_t width)
    -> std::array<size_t, NUM_X_REGIONS>
{
  const uint32_t regionSize = width / NUM_X_REGIONS;

  std::array<size_t, NUM_X_REGIONS> regionBorders{};
  size_t xBorder = regionSize - 1;
  for (auto& regionBorder : regionBorders)
  {
    regionBorder = xBorder;

    xBorder += regionSize;
    if (xBorder >= width)
    {
      xBorder = width - 1;
    }
    assert(xBorder < width);
  }

  return regionBorders;
}

auto FilterBufferColorInfo::GetYRegionBorders(const uint32_t height)
    -> std::array<size_t, NUM_Y_REGIONS>
{
  const uint32_t regionSize = height / NUM_Y_REGIONS;

  std::array<size_t, NUM_Y_REGIONS> regionBorders{};
  size_t yBorder = regionSize - 1;
  for (auto& regionBorder : regionBorders)
  {
    regionBorder = yBorder;

    yBorder += regionSize;
    if (yBorder >= height)
    {
      yBorder = height - 1;
    }
    assert(yBorder < height);
  }

  return regionBorders;
}

auto FilterBufferColorInfo::GetRegionInfoArray(
    const std::array<size_t, NUM_Y_REGIONS>& yRegionBorders) -> std::array<RegionInfo, NUM_REGIONS>
{
  std::array<RegionInfo, NUM_REGIONS> regionRectArray{};

  size_t regionIndex = 0;
  size_t y0 = 0;
  for (const auto& yBorder : yRegionBorders)
  {
    const size_t y1 = yBorder;
    assert((0 <= y0) && (y0 <= y1));

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
    const uint32_t height, const std::array<size_t, NUM_X_REGIONS>& xRegionBorders)
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
    const size_t xRegionIndex) const -> Counts
{
  return m_xRegionCountsArray.at(xRegionIndex);
}

inline auto FilterBufferColorInfo::GetAverageLuminance(const Counts& totals) -> float
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

auto FilterBufferColorInfo::GetRegionAverageLuminance(const size_t regionIndex) const -> float
{
  const RegionInfo& regionInfo = m_regionInfoArray.at(regionIndex);

  Counts totals{};

  for (size_t y = regionInfo.y0; y <= regionInfo.y1; ++y)
  {
    const Counts& xRegionCounts =
        m_filterBufferRowColorInfoArray.at(y).GetXRegionCountsArray().at(regionInfo.xRegionIndex);

    totals.numNonzeroInRow += xRegionCounts.numNonzeroInRow;
    totals.sumRedInRow += xRegionCounts.sumRedInRow;
    totals.sumGreenInRow += xRegionCounts.sumGreenInRow;
    totals.sumBlueInRow += xRegionCounts.sumBlueInRow;
  }

  return GetAverageLuminance(totals);
}

auto FilterBufferColorInfo::GetMaxRegionAverageLuminance() const -> float
{
  std::array<float, NUM_REGIONS> regionAverageLuminances{};
  for (size_t i = 0; i < NUM_REGIONS; ++i)
  {
    regionAverageLuminances.at(i) = GetRegionAverageLuminance(i);
  }

  float maxAverageLuminance = 0.0F;
  for (const float regionAverageLuminance : regionAverageLuminances)
  {
    if (regionAverageLuminance > maxAverageLuminance)
    {
      maxAverageLuminance = regionAverageLuminance;
    }
  }

  return maxAverageLuminance;
}

auto FilterBufferColorInfo::GetAverageLuminanceTest() const -> float
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

auto FilterBufferColorInfo::FilterBufferRowColorInfo::GetXRegionTotals() const -> Counts
{
  const std::array<Counts, NUM_X_REGIONS>& regionCountsArray = GetXRegionCountsArray();

  Counts totals{};
  for (const auto& regionCounts : regionCountsArray)
  {
    totals.numNonzeroInRow += regionCounts.numNonzeroInRow;
    totals.sumRedInRow += regionCounts.sumRedInRow;
    totals.sumGreenInRow += regionCounts.sumGreenInRow;
    totals.sumBlueInRow += regionCounts.sumBlueInRow;
  }

  return totals;
}

} // namespace GOOM::VISUAL_FX::FILTERS
