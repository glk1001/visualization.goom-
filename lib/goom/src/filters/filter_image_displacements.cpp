#include "filter_image_displacements.h"

#include "goom/goom_config.h"
#include "goomutils/goomrand.h"

#undef NDEBUG
#include <cassert>
#include <string>
#include <vector>

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
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.1000F};
constexpr NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
constexpr NumberRange<float> ZOOM_FACTOR_RANGE = {1.00F, 10.0F};

constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

// clang-format off
//@formatter:off
const std::vector<std::string> ImageDisplacements::IMAGE_FILENAMES{
  "pattern1.jpg",
  "pattern2.jpg",
  "pattern3.jpg",
  "pattern4.jpg",
  "pattern5.jpg",
  "chameleon-tail.jpg",
  "mountain_sunset.png",
  };
// clang-format on
//@formatter:on

ImageDisplacements::ImageDisplacements(const std::string& resourcesDirectory)
  : m_resourcesDirectory{resourcesDirectory}
{
  if (m_resourcesDirectory.empty())
  {
    return;
  }

  for (const auto& imageFilename : IMAGE_FILENAMES)
  {
    m_imageDisplacements.emplace_back(GetImageFilename(imageFilename));
  }
  SetRandomParams();
}

auto ImageDisplacements::GetImageFilename(const std::string& imageFilename) const -> std::string
{
  return m_resourcesDirectory + PATH_SEP + IMAGES_DIR + PATH_SEP + IMAGE_DISPLACEMENT_DIR +
         PATH_SEP + imageFilename;
}

auto ImageDisplacements::GetParams() const -> Params
{
  return {GetCurrentImageDisplacement().GetAmplitude(),
          GetCurrentImageDisplacement().GetXColorCutoff(),
          GetCurrentImageDisplacement().GetYColorCutoff(),
          GetCurrentImageDisplacement().GetZoomFactor()};
}

void ImageDisplacements::SetParams(const Params& params)
{
  GetCurrentImageDisplacement().SetAmplitude(params.amplitude);
  GetCurrentImageDisplacement().SetXyColorCutoffs(params.xColorCutoff, params.yColorCutoff);
  GetCurrentImageDisplacement().SetZoomFactor(params.zoomFactor);
}

inline void ImageDisplacements::SetRandomImageDisplacement()
{
  m_currentImageDisplacementIndex =
      GetRandInRange(0U, static_cast<uint32_t>(m_imageDisplacements.size()));
}

void ImageDisplacements::SetRandomParams()
{
  SetRandomImageDisplacement();

  const float xColorCutoff = GetRandInRange(COLOR_CUTOFF_RANGE);

  SetParams({
      GetRandInRange(AMPLITUDE_RANGE),
      xColorCutoff,
      ProbabilityOf(PROB_XY_COLOR_CUTOFFS_EQUAL) ? xColorCutoff
                                                 : GetRandInRange(COLOR_CUTOFF_RANGE),
      GetRandInRange(ZOOM_FACTOR_RANGE),
  });
}

auto ImageDisplacements::GetSpeedCoefficientsEffectNameValueParams() const
    -> std::vector<std::pair<std::string, std::string>>
{
  return std::vector<std::pair<std::string, std::string>>();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
