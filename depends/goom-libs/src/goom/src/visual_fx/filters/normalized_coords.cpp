#include "normalized_coords.h"

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

// clang linking trouble with C++14 - should be able to use static constexpr
const float NormalizedCoords::MAX_NORMALIZED_COORD = 2.0;
const float NormalizedCoords::MIN_NORMALIZED_COORD = -MAX_NORMALIZED_COORD;
float NormalizedCoords::s_ratioScreenToNormalizedCoord = 1.0F;
float NormalizedCoords::s_ratioNormalizedToScreenCoord = 1.0F;
float NormalizedCoords::s_minNormalizedCoordVal = 0.0F;

void NormalizedCoords::SetScreenDimensions(const uint32_t width,
                                           const uint32_t height,
                                           const float minScreenCoordVal)
{
  assert(width > 1);
  assert(height > 1);

  const uint32_t maxDimension = std::max(width, height);
  s_ratioScreenToNormalizedCoord =
      (MAX_NORMALIZED_COORD - MIN_NORMALIZED_COORD) / static_cast<float>(maxDimension - 1);
  s_ratioNormalizedToScreenCoord = 1.0F / s_ratioScreenToNormalizedCoord;
  s_minNormalizedCoordVal = minScreenCoordVal * s_ratioScreenToNormalizedCoord;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
