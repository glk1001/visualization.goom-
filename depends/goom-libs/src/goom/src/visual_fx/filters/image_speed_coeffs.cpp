#include "image_speed_coeffs.h"

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

using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.01000F};
constexpr NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
constexpr NumberRange<float> ZOOM_FACTOR_RANGE = {0.10F, 1.0F};

constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

ImageSpeedCoefficients::ImageSpeedCoefficients(const std::string& resourcesDirectory)
  : m_imageDisplacementList{resourcesDirectory}
{
  if (!resourcesDirectory.empty())
  {
    SetRandomParams();
  }
}

void ImageSpeedCoefficients::SetRandomParams()
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

auto ImageSpeedCoefficients::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  constexpr const char* PARAM_GROUP = "ImageSpeedCoeffs";
  return m_imageDisplacementList.GetNameValueParams(PARAM_GROUP);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
