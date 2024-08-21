module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Noise;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Noise
{
public:
  explicit Noise(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs;

  struct Params
  {
    float noiseFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const GoomRand* m_goomRand;

  // For noise amplitude, take the reciprocal of these.
  static constexpr auto NOISE_RANGE = NumberRange{40.0F, 120.0F};
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto Noise::GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const auto amp = m_params.noiseFactor / m_goomRand->GetRandInRange<NOISE_RANGE>();

  return velocity + NormalizedCoords{m_goomRand->GetRandInRange(NumberRange{-amp, +amp}),
                                     m_goomRand->GetRandInRange(NumberRange{-amp, +amp})};
}

inline auto Noise::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Noise::SetParams(const Params& params) -> void
{
  m_params = params;
}

inline auto Noise::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
