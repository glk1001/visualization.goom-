#pragma once

#include "filter_effects/filter_zoom_vector_effects.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace FILTER_FX
{

class FilterZoomVector : public IZoomVector
{
public:
  FilterZoomVector(uint32_t screenWidth,
                   const std::string& resourcesDirectory,
                   const UTILS::MATH::IGoomRand& goomRand,
                   const NormalizedCoordsConverter& normalizedCoordsConverter,
                   const FILTER_EFFECTS::ZoomVectorEffects::GetTheEffectsFunc& getTheExtraEffects =
                       FILTER_EFFECTS::ZoomVectorEffects::GetStandardExtraEffects) noexcept;

  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) -> void override;

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const
      -> NormalizedCoords override;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs override;

private:
  FILTER_EFFECTS::ZoomVectorEffects m_zoomVectorEffects;

  [[nodiscard]] auto GetZoomEffectsAdjustedVelocity(float sqDistFromZero,
                                                    const NormalizedCoords& coords,
                                                    const NormalizedCoords& velocity) const
      -> NormalizedCoords;
};

} // namespace FILTER_FX
} // namespace GOOM

