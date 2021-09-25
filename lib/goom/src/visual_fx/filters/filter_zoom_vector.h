#pragma once

#include "filter_zoom_vector_effects.h"
#include "goomutils/name_value_pairs.h"
#include "normalized_coords.h"
#include "v2d.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

namespace FILTERS
{

class FilterZoomVector : public IZoomVector
{
public:
  explicit FilterZoomVector(uint32_t screenWidth, const std::string& resourcesDirectory) noexcept;

  void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) override;

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords override;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs override;

private:
  ZoomVectorEffects m_zoomVectorEffects;

  [[nodiscard]] auto GetZoomEffectsAdjustedVelocity(float sqDistFromZero,
                                                    const NormalizedCoords& coords,
                                                    const NormalizedCoords& velocity) const
      -> NormalizedCoords;
};

} // namespace FILTERS
} // namespace GOOM

