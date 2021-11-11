#pragma once

#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <cmath>

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

class TanEffect
{
public:
  explicit TanEffect(UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams();

  [[nodiscard]] auto GetVelocity(float sqDistFromZero, const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
    float limitingFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  UTILS::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto TanEffect::GetVelocity(const float sqDistFromZero,
                                   const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const float limit = m_params.limitingFactor * UTILS::m_half_pi;
  const float tanArg = stdnew::clamp(std::fmod(sqDistFromZero, UTILS::m_half_pi), -limit, +limit);
  const float tanSqDist = std::tan(tanArg);
  return {m_params.xAmplitude * tanSqDist * velocity.GetX(),
          m_params.yAmplitude * tanSqDist * velocity.GetY()};
}

inline auto TanEffect::GetParams() const -> const Params&
{
  return m_params;
}

inline void TanEffect::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
