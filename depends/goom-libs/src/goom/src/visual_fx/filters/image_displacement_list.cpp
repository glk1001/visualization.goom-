#include "image_displacement_list.h"

#include "goom/goom_config.h"
#include "utils/randutils.h"
#include "utils/name_value_pairs.h"

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

using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;

// clang-format off
//@formatter:off
const std::vector<std::string> ImageDisplacementList::IMAGE_FILENAMES{
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

ImageDisplacementList::ImageDisplacementList(const std::string& resourcesDirectory)
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
}

void ImageDisplacementList::SetRandomImageDisplacement()
{
  m_currentImageDisplacementIndex =
      GetRandInRange(0U, static_cast<uint32_t>(m_imageDisplacements.size()));
}

auto ImageDisplacementList::GetImageFilename(const std::string& imageFilename) const -> std::string
{
  return m_resourcesDirectory + PATH_SEP + IMAGES_DIR + PATH_SEP + IMAGE_DISPLACEMENT_DIR +
         PATH_SEP + imageFilename;
}

auto ImageDisplacementList::GetParams() const -> Params
{
  return {GetCurrentImageDisplacement().GetAmplitude(),
          GetCurrentImageDisplacement().GetXColorCutoff(),
          GetCurrentImageDisplacement().GetYColorCutoff(),
          GetCurrentImageDisplacement().GetZoomFactor()};
}

void ImageDisplacementList::SetParams(const Params& params)
{
  GetCurrentImageDisplacement().SetAmplitude(params.amplitude);
  GetCurrentImageDisplacement().SetXyColorCutoffs(params.xColorCutoff, params.yColorCutoff);
  GetCurrentImageDisplacement().SetZoomFactor(params.zoomFactor);
}

auto ImageDisplacementList::GetNameValueParams(const std::string& paramGroup) const
    -> NameValuePairs
{
  return {
      GetPair(paramGroup, "filename", IMAGE_FILENAMES[m_currentImageDisplacementIndex]),
      GetPair(paramGroup, "zoom factor", GetCurrentImageDisplacement().GetZoomFactor()),
      GetPair(paramGroup, "amplitude", GetCurrentImageDisplacement().GetAmplitude()),
      GetPair(paramGroup, "x cutoff", GetCurrentImageDisplacement().GetXColorCutoff()),
      GetPair(paramGroup, "y cutoff", GetCurrentImageDisplacement().GetYColorCutoff()),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
