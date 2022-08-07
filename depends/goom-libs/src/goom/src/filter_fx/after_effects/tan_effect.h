#pragma once

#include "filter_fx/normalized_coords.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class TanEffect
{
public:
  explicit TanEffect(const GOOM::UTILS::MATH::IGoomRand& goomRand);
  TanEffect(const TanEffect&) noexcept           = delete;
  TanEffect(TanEffect&&) noexcept                = delete;
  virtual ~TanEffect() noexcept                  = default;
  auto operator=(const TanEffect&) -> TanEffect& = delete;
  auto operator=(TanEffect&&) -> TanEffect&      = delete;

  virtual auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(float sqDistFromZero, const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> GOOM::UTILS::NameValuePairs;

  enum class TanType
  {
    TAN_ONLY,
    COT_ONLY,
    COT_MIX,
    _num // unused, and marks the enum end
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
  auto SetParams(const Params& params) -> void;

private:
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  const GOOM::UTILS::MATH::Weights<TanType> m_tanEffectWeights;
  [[nodiscard]] auto GetTanSqDist(float tanArg) const -> float;
};

inline auto TanEffect::GetVelocity(const float sqDistFromZero,
                                   const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const auto limit = m_params.limitingFactor * GOOM::UTILS::MATH::HALF_PI;
  const auto tanArg =
      std::clamp(std::fmod(sqDistFromZero, GOOM::UTILS::MATH::HALF_PI), -limit, +limit);
  const auto tanSqDist = GetTanSqDist(tanArg);
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
      return std::tan(GOOM::UTILS::MATH::HALF_PI - tanArg);
    case TanType::COT_MIX:
      return std::tan((m_params.cotMix * GOOM::UTILS::MATH::HALF_PI) - tanArg);
    default:
      throw std::logic_error("Unknown TanType enum.");
  }
}

inline auto TanEffect::GetParams() const -> const Params&
{
  return m_params;
}

inline auto TanEffect::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
