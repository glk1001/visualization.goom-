#ifndef VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H

#include "v2d.h"

#include <memory>

namespace GOOM
{

namespace FILTERS
{

class NormalizedCoords;
class SpeedCoefficientsEffect;
struct ZoomFilterSettings;

class IZoomVector
{
public:
  IZoomVector() noexcept = default;
  IZoomVector(const IZoomVector&) noexcept = delete;
  IZoomVector(IZoomVector&&) noexcept = delete;
  virtual ~IZoomVector() noexcept = default;
  auto operator=(const IZoomVector&) -> IZoomVector& = delete;
  auto operator=(IZoomVector&&) -> IZoomVector& = delete;

  virtual void SetMaxSpeedCoeff(float val) = 0;

  virtual void SetFilterSettings(const ZoomFilterSettings& filterSettings) = 0;

  virtual auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords = 0;
};

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H
