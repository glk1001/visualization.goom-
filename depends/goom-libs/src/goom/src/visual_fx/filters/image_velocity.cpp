#include "image_velocity.h"

#include "utils/randutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetFullParamGroup;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.0100F};
constexpr NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
constexpr NumberRange<float> ZOOM_FACTOR_RANGE = {0.9F, 1.0F};

constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

ImageVelocity::ImageVelocity(const std::string& resourcesDirectory)
  : m_imageDisplacementList{resourcesDirectory}
{
  if (!resourcesDirectory.empty())
  {
    SetRandomParams();
  }
}

void ImageVelocity::SetRandomParams()
{
  m_imageDisplacementList.SetRandomImageDisplacement();

  const float xColorCutoff = GetRandInRange(COLOR_CUTOFF_RANGE);

  m_imageDisplacementList.SetParams({
      GetRandInRange(AMPLITUDE_RANGE),
      xColorCutoff,
      ProbabilityOf(PROB_XY_COLOR_CUTOFFS_EQUAL) ? xColorCutoff
                                                 : GetRandInRange(COLOR_CUTOFF_RANGE),
      GetRandInRange(ZOOM_FACTOR_RANGE),
  });
}

auto ImageVelocity::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  return m_imageDisplacementList.GetNameValueParams(
      GetFullParamGroup({paramGroup, "ImageVelocity"}));
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
