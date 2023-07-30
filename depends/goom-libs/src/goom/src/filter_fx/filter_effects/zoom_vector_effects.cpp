#include "zoom_vector_effects.h"

#include "filter_fx/after_effects/zoom_vector_after_effects.h"
#include "filter_fx/filter_settings.h"
#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using AFTER_EFFECTS::AfterEffects;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory,
                                     const IGoomRand& goomRand,
                                     const GetAfterEffectsFunc& getAfterEffects) noexcept
  : m_zoomVectorAfterEffects{screenWidth, getAfterEffects(goomRand, resourcesDirectory)}
{
  static_assert(MIN_ALLOWED_BASE_ZOOM_ADJUSTMENT <=
                GetBaseZoomAdjustment(RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, -1.0F));
  static_assert(MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT >=
                GetBaseZoomAdjustment(RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, +1.0F));
}

auto ZoomVectorEffects::GetStandardAfterEffects(const IGoomRand& goomRand,
                                                const std::string& resourcesDirectory) noexcept
    -> AfterEffects
{
  return AFTER_EFFECTS::GetStandardAfterEffects(goomRand, resourcesDirectory);
}

auto ZoomVectorEffects::SetFilterSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->zoomAdjustmentEffect->SetRandomParams();

  SetBaseZoomAdjustmentFactor(m_filterEffectsSettings->baseZoomAdjustmentFactorMultiplier);

  m_zoomVectorAfterEffects.SetAfterEffectsSettings(m_filterEffectsSettings->afterEffectsSettings,
                                                   m_filterEffectsSettings->zoomMidpoint);
}

auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const noexcept -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "ZoomEffects";

  auto nameValuePairs = UTILS::NameValuePairs{
      GetPair(PARAM_GROUP, "coeffFactor", m_baseZoomAdjustmentFactor),
  };

  UTILS::MoveNameValuePairs(GetZoomAdjustmentNameValueParams(), nameValuePairs);
  UTILS::MoveNameValuePairs(m_zoomVectorAfterEffects.GetZoomEffectsNameValueParams(),
                            nameValuePairs);

  return nameValuePairs;
}

auto ZoomVectorEffects::GetZoomAdjustmentNameValueParams() const noexcept -> NameValuePairs
{
  return m_filterEffectsSettings->zoomAdjustmentEffect->GetZoomAdjustmentEffectNameValueParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
