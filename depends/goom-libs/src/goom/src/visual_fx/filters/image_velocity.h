#pragma once

#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

class ImageVelocity
{
public:
  ImageVelocity(const std::string& resourcesDirectory, UTILS::IGoomRand& goomRand);

  void SetRandomParams();

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const -> V2dFlt;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  UTILS::IGoomRand& m_goomRand;
  ImageDisplacementList m_imageDisplacementList;
};

inline auto ImageVelocity::GetVelocity(const NormalizedCoords& coords) const -> V2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(
      coords.ToFlt());
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
