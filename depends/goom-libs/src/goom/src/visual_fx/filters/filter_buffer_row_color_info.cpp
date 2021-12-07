#include "filter_buffer_row_color_info.h"

#include "color/colorutils.h"

#include <numeric>

namespace GOOM::VISUAL_FX::FILTERS
{

using COLOR::LUMA_BLUE_COMPONENT;
using COLOR::LUMA_GREEN_COMPONENT;
using COLOR::LUMA_RED_COMPONENT;

auto FilterBufferRowColorInfo::GetBufferAverageLuminance(
    const std::vector<FilterBufferRowColorInfo>& filterBufferColorInfo) -> float
{
  const auto infoBegin = cbegin(filterBufferColorInfo);
  const auto infoEnd = cend(filterBufferColorInfo);

  const auto totalNonZeroPixels =
      std::accumulate(infoBegin, infoEnd, 0UL,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_numNonzeroInRow; });

  if (0 == totalNonZeroPixels)
  {
    return 0.0F;
  }

  const uint64_t totalRedColor =
      std::accumulate(infoBegin, infoEnd, 0UL,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumRedInRow; });
  const uint64_t totalGreenColor =
      std::accumulate(infoBegin, infoEnd, 0UL,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumGreenInRow; });
  const uint64_t totalBlueColor =
      std::accumulate(infoBegin, infoEnd, 0UL,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumBlueInRow; });

  return ((LUMA_RED_COMPONENT * static_cast<float>(totalRedColor)) +
          (LUMA_GREEN_COMPONENT * static_cast<float>(totalGreenColor)) +
          (LUMA_BLUE_COMPONENT * static_cast<float>(totalBlueColor))) /
         (static_cast<float>(totalNonZeroPixels) * channel_limits<float>::max());
}

} // namespace GOOM::VISUAL_FX::FILTERS
