#pragma once

#include "goomutils/name_value_pairs.h"
#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "v2d.h"

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

class ImageVelocity
{
public:
  explicit ImageVelocity(const std::string& resourcesDirectory);

  void SetRandomParams();

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const -> V2dFlt;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  ImageDisplacementList m_imageDisplacementList;
};

inline auto ImageVelocity::GetVelocity(const NormalizedCoords& coords) const -> V2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(
      coords.ToFlt());
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

