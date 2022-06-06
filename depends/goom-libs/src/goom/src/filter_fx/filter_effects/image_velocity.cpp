#include "image_velocity.h"

#include "utils/name_value_pairs.h"

#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.0500F};
static constexpr IGoomRand::NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
static constexpr IGoomRand::NumberRange<float> ZOOM_FACTOR_RANGE = {0.5F, 1.0F};

static constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

ImageVelocity::ImageVelocity(const std::string& resourcesDirectory, const IGoomRand& goomRand)
  : m_goomRand{goomRand}, m_imageDisplacementList{resourcesDirectory, m_goomRand}
{
  if (!resourcesDirectory.empty())
  {
    SetRandomParams();
  }
}

auto ImageVelocity::SetRandomParams() -> void
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

auto ImageVelocity::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  return m_imageDisplacementList.GetNameValueParams(
      GetFullParamGroup({paramGroup, "ImageVelocity"}));
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS