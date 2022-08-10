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

class XYLerpEffect
{
public:
  explicit XYLerpEffect(const GOOM::UTILS::MATH::IGoomRand& goomRand);
  XYLerpEffect(const XYLerpEffect&) noexcept           = delete;
  XYLerpEffect(XYLerpEffect&&) noexcept                = delete;
  virtual ~XYLerpEffect() noexcept                     = default;
  auto operator=(const XYLerpEffect&) -> XYLerpEffect& = delete;
  auto operator=(XYLerpEffect&&) -> XYLerpEffect&      = delete;

  virtual auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(float sqDistFromZero, const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> GOOM::UTILS::NameValuePairs;

  enum class Modes
  {
    MODE0,
    MODE1,
    MODE2,
    MODE3,
    MODE4,
    MODE5,
    _num // unused, and marks the enum end
  };
  struct Params
  {
    Modes mode;
    float tFreq;
    bool flipY;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  const GOOM::UTILS::MATH::Weights<Modes> m_modeWeights;
  Params m_params;
  [[nodiscard]] auto GetT(float sqDistFromZero, const NormalizedCoords& velocity) const -> float;
  [[nodiscard]] static auto GetFlipYProbability(Modes mode) -> float;
};

inline auto XYLerpEffect::GetVelocity([[maybe_unused]] const float sqDistFromZero,
                                      const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const auto t = GetT(sqDistFromZero, velocity);

  if (m_params.flipY)
  {
    return {
        +STD20::lerp(velocity.GetX(), velocity.GetY(), t),
        -STD20::lerp(velocity.GetY(), velocity.GetX(), t),
    };
  }

  return {
      STD20::lerp(velocity.GetX(), velocity.GetY(), t),
      STD20::lerp(velocity.GetY(), velocity.GetX(), t),
  };
}

inline auto XYLerpEffect::GetT(const float sqDistFromZero, const NormalizedCoords& velocity) const
    -> float
{
  static constexpr auto MODE2_OFFSET = 5.5F;
  static constexpr auto MODE2_FREQ   = 2.0F;
  static constexpr auto MODE3_MIN_FACTOR = 0.95F;
  static constexpr auto MODE3_MAX_FACTOR = 1.05F;

  switch (m_params.mode)
  {
    case Modes::MODE0:
      return 0.0F;
    case Modes::MODE1:
      return 1.0F;
    case Modes::MODE2:
      return std::cos((m_params.tFreq * sqDistFromZero) +
                      (MODE2_OFFSET + std::sin(MODE2_FREQ * sqDistFromZero)));
    case Modes::MODE3:
      return std::cos((m_params.tFreq * sqDistFromZero) *
                      m_goomRand.GetRandInRange(MODE3_MIN_FACTOR, MODE3_MAX_FACTOR));
    case Modes::MODE4:
      return -(2.0F / STD20::pi) *
             std::atan(std::tan(GOOM::UTILS::MATH::HALF_PI - (m_params.tFreq * sqDistFromZero)));
    case Modes::MODE5:
      return std::abs(std::atan2(velocity.GetY(), velocity.GetX()) / STD20::pi);
    default:
      FailFast();
      return 0.0F;
  }
}

inline auto XYLerpEffect::GetParams() const -> const Params&
{
  return m_params;
}

inline auto XYLerpEffect::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
