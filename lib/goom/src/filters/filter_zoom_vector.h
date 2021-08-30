#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H

#include "filter_normalized_coords.h"
#include "goom_zoom_vector.h"
#include "v2d.h"

#include <memory>

namespace GOOM
{

struct ZoomFilterData;

namespace FILTERS
{

class ZoomVectorEffects;

class FilterZoomVector : public IZoomVector
{
public:
  explicit FilterZoomVector(const std::string& resourcesDirectory) noexcept;
  FilterZoomVector(const FilterZoomVector&) noexcept = delete;
  FilterZoomVector(FilterZoomVector&&) noexcept = delete;
  ~FilterZoomVector() noexcept override;
  auto operator=(const FilterZoomVector&) -> FilterZoomVector& = delete;
  auto operator=(FilterZoomVector&&) -> FilterZoomVector& = delete;

  auto GetFilterSettings() const -> const ZoomFilterData* override;
  void SetFilterSettings(const ZoomFilterData& filterSettings) override;
  void SetRandomPlaneEffects(const V2dInt& zoomMidPoint, uint32_t screenWidth) override;

  auto GetMaxSpeedCoeff() const -> float override;
  void SetMaxSpeedCoeff(float val) override;

  auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords override;

private:
  const ZoomFilterData* m_filterSettings{};
  std::unique_ptr<ZoomVectorEffects> m_zoomVectorEffects;

  void GetZoomEffectsAdjustedVelocity(float sqDistFromZero,
                                      const NormalizedCoords& coords,
                                      NormalizedCoords& velocity) const;
};

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
