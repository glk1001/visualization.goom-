#include "normalized_coords.h"

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <cstdint>

namespace GOOM::VISUAL_FX::FILTERS
{

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

} // namespace GOOM::VISUAL_FX::FILTERS
