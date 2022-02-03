#pragma once

#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#include <cmath>

namespace GOOM::VISUAL_FX::FILTERS
{

class TanEffect
{
public:
  explicit TanEffect(const UTILS::IGoomRand& goomRand) noexcept;
  TanEffect(const TanEffect&) noexcept = delete;
  TanEffect(TanEffect&&) noexcept = delete;
  virtual ~TanEffect() noexcept = default;
  auto operator=(const TanEffect&) -> TanEffect& = delete;
  auto operator=(TanEffect&&) -> TanEffect& = delete;

  virtual void SetRandomParams();

  [[nodiscard]] auto GetVelocity(float sqDistFromZero, const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
    float limitingFactor;
    bool useTan;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
};

inline auto TanEffect::GetVelocity(const float sqDistFromZero,
                                   const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const float limit = m_params.limitingFactor * UTILS::m_half_pi;
  const float tanArg = std::clamp(std::fmod(sqDistFromZero, UTILS::m_half_pi), -limit, +limit);
  const float tanSqDist = m_params.useTan ? std::tan(tanArg) : std::tan(UTILS::m_half_pi - tanArg);
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

} // namespace GOOM::VISUAL_FX::FILTERS
