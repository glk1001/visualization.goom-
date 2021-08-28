#ifndef VISUALIZATION_GOOM_FILTER_IMAGE_DISPLACEMENTS_H
#define VISUALIZATION_GOOM_FILTER_IMAGE_DISPLACEMENTS_H

#include "goomutils/goomrand.h"
#include "image_displacement.h"
#include "v2d.h"

#include <memory>
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

class ImageDisplacements
{
public:
  explicit ImageDisplacements(const std::string& resourcesDirectory) noexcept;

  auto GetDisplacementVector(const V2dFlt& normalizedPoint) const -> V2dFlt;

  auto GetCurrentImageFilename() const -> std::string;

  void SetRandomParams();

  struct Params
  {
    float amplitude;
    float xColorCutoff;
    float yColorCutoff;
    float zoomFactor;
  };
  [[nodiscard]] auto GetParams() const -> Params;

protected:
  void SetParams(const Params& params);

private:
  static const std::vector<std::string> IMAGE_FILENAMES;
  [[nodiscard]] auto GetImageFilename(const std::string& imageFilename) const -> std::string;

  std::vector<ImageDisplacement> m_imageDisplacements{};
  const std::string m_resourcesDirectory;
  size_t m_currentImageDisplacementIndex = 0;
  auto GetCurrentImageDisplacement() const -> const ImageDisplacement&;
  auto GetCurrentImageDisplacement() -> ImageDisplacement&;
  void SetRandomImageDisplacement();
};

inline auto ImageDisplacements::GetCurrentImageDisplacement() const -> const ImageDisplacement&
{
  return m_imageDisplacements[m_currentImageDisplacementIndex];
}

inline auto ImageDisplacements::GetCurrentImageDisplacement() -> ImageDisplacement&
{
  return m_imageDisplacements[m_currentImageDisplacementIndex];
}

inline auto ImageDisplacements::GetDisplacementVector(const V2dFlt& normalizedPoint) const -> V2dFlt
{
  return GetCurrentImageDisplacement().GetDisplacementVector(normalizedPoint);
}

inline auto ImageDisplacements::GetCurrentImageFilename() const -> std::string
{
  return GetCurrentImageDisplacement().GetImageFilename();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_IMAGE_DISPLACEMENTS_H
