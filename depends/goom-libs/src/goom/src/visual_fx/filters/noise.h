#pragma once

#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Noise
{
public:
  explicit Noise(UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams();

  [[nodiscard]] auto GetVelocity() const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float noiseFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  UTILS::IGoomRand& m_goomRand;
  // For noise amplitude, take the reciprocal of these.
  static constexpr float NOISE_MIN = 40.0F;
  static constexpr float NOISE_MAX = 120.0F;
  Params m_params;
};

inline auto Noise::GetVelocity() const -> NormalizedCoords
{
  const float amp = m_params.noiseFactor / m_goomRand.GetRandInRange(NOISE_MIN, NOISE_MAX);
  return {m_goomRand.GetRandInRange(-amp, +amp), m_goomRand.GetRandInRange(-amp, +amp)};
}

inline auto Noise::GetParams() const -> const Params&
{
  return m_params;
}

inline void Noise::SetParams(const Params& params)
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
