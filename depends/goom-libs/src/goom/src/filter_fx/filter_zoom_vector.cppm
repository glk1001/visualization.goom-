module;

#include <cstdint>
#include <string>

export module Goom.FilterFx.FilterZoomVector;

import Goom.FilterFx.FilterEffects.ZoomVectorEffects;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomVector;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

export namespace GOOM::FILTER_FX
{

class FilterZoomVector : public IZoomVector
{
public:
  FilterZoomVector(uint32_t screenWidth,
                   const std::string& resourcesDirectory,
                   const UTILS::MATH::GoomRand& goomRand,
                   const FILTER_EFFECTS::ZoomVectorEffects::GetAfterEffectsFunc& getAfterEffects =
                       FILTER_EFFECTS::ZoomVectorEffects::GetStandardAfterEffects) noexcept;

  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void override;

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const noexcept
      -> NormalizedCoords override;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs override;

private:
  FILTER_EFFECTS::ZoomVectorEffects m_zoomVectorEffects;
  [[nodiscard]] auto GetFilterEffectsZoomPoint(const NormalizedCoords& coords) const noexcept
      -> NormalizedCoords;
  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             const NormalizedCoords& zoomPoint) const noexcept
      -> NormalizedCoords;
};

} // namespace GOOM::FILTER_FX
