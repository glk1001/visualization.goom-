#ifndef VISUALIZATION_GOOM_LIB_FILTERS_FILTER_IMAGE_DISPLACEMENT_LIST_H
#define VISUALIZATION_GOOM_LIB_FILTERS_FILTER_IMAGE_DISPLACEMENT_LIST_H

#include "goomutils/name_value_pairs.h"
#include "image_displacement.h"

#include <cstddef>
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

class ImageDisplacementList
{
public:
  explicit ImageDisplacementList(const std::string& resourcesDirectory);

  struct Params
  {
    float amplitude;
    float xColorCutoff;
    float yColorCutoff;
    float zoomFactor;
  };
  [[nodiscard]] auto GetParams() const -> Params;
  void SetParams(const Params& params);
  void SetRandomImageDisplacement();
  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  [[nodiscard]] auto GetCurrentImageDisplacement() const -> const ImageDisplacement&;
  [[nodiscard]] auto GetCurrentImageDisplacement() -> ImageDisplacement&;

private:
  static const std::vector<std::string> IMAGE_FILENAMES;

  std::vector<ImageDisplacement> m_imageDisplacements{};
  const std::string m_resourcesDirectory;
  size_t m_currentImageDisplacementIndex = 0;
  [[nodiscard]] auto GetImageFilename(const std::string& imageFilename) const -> std::string;
};

inline auto ImageDisplacementList::GetCurrentImageDisplacement() const -> const ImageDisplacement&
{
  return m_imageDisplacements[m_currentImageDisplacementIndex];
}

inline auto ImageDisplacementList::GetCurrentImageDisplacement() -> ImageDisplacement&
{
  return m_imageDisplacements[m_currentImageDisplacementIndex];
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_LIB_FILTERS_FILTER_IMAGE_DISPLACEMENT_LIST_H
