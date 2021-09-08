#ifndef VISUALIZATION_GOOM_FILTER_IMAGE_DISPLACEMENTS_H
#define VISUALIZATION_GOOM_FILTER_IMAGE_DISPLACEMENTS_H

#include "filter_speed_coefficients_effect.h"
#include "goomutils/goomrand.h"
#include "image_displacement.h"
#include "v2d.h"

#include <cstddef>
#include <memory>
#include <string>
#include <tuple>
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

class ImageDisplacements : public ISpeedCoefficientsEffect
{
public:
  explicit ImageDisplacements(const std::string& resourcesDirectory);

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> std::vector<std::pair<std::string, std::string>> override;

  auto GetCurrentImageFilename() const -> std::string;

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

inline auto ImageDisplacements::GetSpeedCoefficients([[maybe_unused]] const V2dFlt& baseSpeedCoeffs,
                                                     [[maybe_unused]] float sqDistFromZero,
                                                     const NormalizedCoords& coords) const -> V2dFlt
{
  return GetCurrentImageDisplacement().GetDisplacementVector(coords.ToFlt());
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
