#include "image_speed_coeffs.h"

#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.01000F};
static constexpr IGoomRand::NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
static constexpr IGoomRand::NumberRange<float> ZOOM_FACTOR_RANGE = {0.10F, 1.0F};

static constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

ImageSpeedCoefficients::ImageSpeedCoefficients(const std::string& resourcesDirectory,
                                               const IGoomRand& goomRand)
  : m_goomRand{goomRand}, m_imageDisplacementList{resourcesDirectory, m_goomRand}
{
  if (!resourcesDirectory.empty())
  {
    DoSetRandomParams();
  }
}

auto ImageSpeedCoefficients::SetRandomParams() -> void
{
  DoSetRandomParams();
}

inline auto ImageSpeedCoefficients::DoSetRandomParams() -> void
{
  m_imageDisplacementList.SetRandomImageDisplacement();

  const float xColorCutoff = m_goomRand.GetRandInRange(COLOR_CUTOFF_RANGE);

  m_imageDisplacementList.SetParams({
      m_goomRand.GetRandInRange(AMPLITUDE_RANGE),
      xColorCutoff,
      m_goomRand.ProbabilityOf(PROB_XY_COLOR_CUTOFFS_EQUAL)
          ? xColorCutoff
          : m_goomRand.GetRandInRange(COLOR_CUTOFF_RANGE),
      m_goomRand.GetRandInRange(ZOOM_FACTOR_RANGE),
  });
}

auto ImageSpeedCoefficients::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  static constexpr const char* PARAM_GROUP = "ImageSpeedCoeffs";
  return m_imageDisplacementList.GetNameValueParams(PARAM_GROUP);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS