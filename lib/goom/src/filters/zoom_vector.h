#ifndef VISUALIZATION_GOOM_LIB_FILTERS_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_LIB_FILTERS_ZOOM_VECTOR_H

#include "goomutils/name_value_pairs.h"
#include "v2d.h"

#include <string>

namespace GOOM
{

namespace FILTERS
{

class NormalizedCoords;
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

  virtual auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords = 0;

  [[nodiscard]] virtual auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs = 0;
};

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_LIB_FILTERS_ZOOM_VECTOR_H
