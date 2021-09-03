#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H

#include "filter_normalized_coords.h"
#include "filter_zoom_vector_effects.h"
#include "goom_zoom_vector.h"
#include "v2d.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

struct ZoomFilterSettings;

namespace FILTERS
{

class FilterZoomVector : public IZoomVector
{
public:
  explicit FilterZoomVector(uint32_t screenWidth, const std::string& resourcesDirectory) noexcept;

  void SetMaxSpeedCoeff(float val) override;

  void SetFilterSettings(const ZoomFilterSettings& filterSettings) override;

  auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords override;

private:
  ZoomVectorEffects m_zoomVectorEffects;

  void GetZoomEffectsAdjustedVelocity(float sqDistFromZero,
                                      const NormalizedCoords& coords,
                                      NormalizedCoords& velocity) const;
};

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
