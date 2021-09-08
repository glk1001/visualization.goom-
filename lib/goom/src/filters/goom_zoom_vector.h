#ifndef VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H

#include "v2d.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace GOOM
{

namespace FILTERS
{

class NormalizedCoords;
class ISpeedCoefficientsEffect;
struct ZoomFilterEffectsSettings;

class IZoomVector
{
public:
  IZoomVector() noexcept = default;
  IZoomVector(const IZoomVector&) noexcept = delete;
  IZoomVector(IZoomVector&&) noexcept = delete;
  virtual ~IZoomVector() noexcept = default;
  auto operator=(const IZoomVector&) -> IZoomVector& = delete;
  auto operator=(IZoomVector&&) -> IZoomVector& = delete;

  virtual void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) = 0;
  virtual void SetMaxSpeedCoeff(float val) = 0;

  virtual auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords = 0;

  [[nodiscard]] virtual auto GetNameValueParams(const std::string& paramGroup) const
      -> std::vector<std::pair<std::string, std::string>> = 0;
};

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_GOOM_ZOOM_VECTOR_H
