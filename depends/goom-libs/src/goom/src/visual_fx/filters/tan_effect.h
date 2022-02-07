#pragma once

#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

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

  enum class TanType
  {
    TAN_ONLY,
    COT_ONLY,
    COT_MIX,
    _NUM
  };
  struct Params
  {
    TanType tanType;
    float cotMix;
    float xAmplitude;
    float yAmplitude;
    float limitingFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetTanSqDist(float tanArg) const -> float;
};

inline auto TanEffect::GetVelocity(const float sqDistFromZero,
                                   const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const float limit = m_params.limitingFactor * UTILS::m_half_pi;
  const float tanArg = std::clamp(std::fmod(sqDistFromZero, UTILS::m_half_pi), -limit, +limit);
  const float tanSqDist = GetTanSqDist(tanArg);
  return {m_params.xAmplitude * tanSqDist * velocity.GetX(),
          m_params.yAmplitude * tanSqDist * velocity.GetY()};
}

inline auto TanEffect::GetTanSqDist(const float tanArg) const -> float
{
  switch (m_params.tanType)
  {
    case TanType::TAN_ONLY:
      return std::tan(tanArg);
    case TanType::COT_ONLY:
      return std::tan(UTILS::m_half_pi - tanArg);
    case TanType::COT_MIX:
      return std::tan((m_params.cotMix * UTILS::m_half_pi) - tanArg);
    default:
      throw std::logic_error("Unknown TanType enum.");
  }
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
