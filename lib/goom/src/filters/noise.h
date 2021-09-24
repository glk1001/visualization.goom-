#ifndef VVISUALIZATION_GOOM_LIB_FILTERS_NOISE_H
#define VISUALIZATION_GOOM_LIB_FILTERS_NOISE_H

#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"
#include "normalized_coords.h"
#include "v2d.h"

#include <cmath>

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
  Noise() noexcept;

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
  // For noise amplitude, take the reciprocal of these.
  static constexpr float NOISE_MIN = 40.0F;
  static constexpr float NOISE_MAX = 120.0F;
  Params m_params;
};

inline auto Noise::GetVelocity() const -> NormalizedCoords
{
  const float amp = m_params.noiseFactor / UTILS::GetRandInRange(NOISE_MIN, NOISE_MAX);
  return {UTILS::GetRandInRange(-amp, +amp), UTILS::GetRandInRange(-amp, +amp)};
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
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_LIB_FILTERS_NOISE_H
