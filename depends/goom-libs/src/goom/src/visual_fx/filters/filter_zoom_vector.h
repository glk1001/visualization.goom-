#pragma once

#include "filter_zoom_vector_effects.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS
{
class IGoomRand;
} // namespace UTILS

namespace VISUAL_FX
{
namespace FILTERS
{

class FilterZoomVector : public IZoomVector
{
public:
  FilterZoomVector(uint32_t screenWidth,
                   const std::string& resourcesDirectory,
                   const UTILS::IGoomRand& goomRand,
                   const ZoomVectorEffects::GetTheEffectsFunc& getTheExtraEffects =
                       ZoomVectorEffects::GetStandardExtraEffects) noexcept;

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
} // namespace VISUAL_FX
} // namespace GOOM

