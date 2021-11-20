#include "filter_buffer_row_color_info.h"

#include "color/colorutils.h"

#include <numeric>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using COLOR::LUMA_BLUE_COMPONENT;
using COLOR::LUMA_GREEN_COMPONENT;
using COLOR::LUMA_RED_COMPONENT;

auto FilterBufferRowColorInfo::GetBufferAverageLuminance(
    const std::vector<FilterBufferRowColorInfo>& filterBufferColorInfo) -> float
{
  auto infoBegin = cbegin(filterBufferColorInfo);
  auto infoEnd = cend(filterBufferColorInfo);

  const auto totalNonZeroPixels =
      std::accumulate(infoBegin, infoEnd, 0,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_numNonzeroInRow; });

  if (totalNonZeroPixels == 0)
  {
    return 0.0F;
  }

  const auto totalRedColor =
      std::accumulate(infoBegin, infoEnd, 0,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumRedInRow; });
  const auto totalGreenColor =
      std::accumulate(infoBegin, infoEnd, 0,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumGreenInRow; });
  const auto totalBlueColor =
      std::accumulate(infoBegin, infoEnd, 0,
                      [](const uint64_t sum, const FilterBufferRowColorInfo& info)
                      { return sum + info.m_sumBlueInRow; });

  return ((LUMA_RED_COMPONENT * static_cast<float>(totalRedColor)) +
          (LUMA_GREEN_COMPONENT * static_cast<float>(totalGreenColor)) +
          (LUMA_BLUE_COMPONENT * static_cast<float>(totalBlueColor))) /
         (static_cast<float>(totalNonZeroPixels) * channel_limits<float>::max());
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
