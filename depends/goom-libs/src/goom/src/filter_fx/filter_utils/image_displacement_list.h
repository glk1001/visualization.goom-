#pragma once

#include "filter_fx/common_types.h"
#include "image_displacement.h"

#include <cstddef>
#include <string>
#include <vector>

import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.Misc;

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ImageDisplacementList
{
public:
  ImageDisplacementList(const std::string& resourcesDirectory,
                        const UTILS::MATH::IGoomRand& goomRand);

  struct Params
  {
    Amplitude amplitude;
    float xColorCutoff;
    float yColorCutoff;
    float xZoomFactor;
    float yZoomFactor;
  };
  [[nodiscard]] auto GetParams() const -> Params;
  auto SetParams(const Params& params) -> void;
  auto SetRandomImageDisplacement() -> void;
  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  [[nodiscard]] auto GetCurrentImageDisplacement() const -> const ImageDisplacement&;
  [[nodiscard]] auto GetCurrentImageDisplacement() -> ImageDisplacement&;

private:
  std::vector<ImageDisplacement> m_imageDisplacements{};
  std::string m_resourcesDirectory;
  const UTILS::MATH::IGoomRand* m_goomRand;
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

} // namespace GOOM::FILTER_FX::FILTER_UTILS
