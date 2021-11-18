#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

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

class DistanceField : public ISpeedCoefficientsEffect
{
public:
  explicit DistanceField(UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    bool mode0;
    float xAmplitude;
    float xSqDistMult;
    float xSqDistOffset;
    float yAmplitude;
    float ySqDistMult;
    float ySqDistOffset;
    std::vector<NormalizedCoords> distancePoints;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  UTILS::IGoomRand& m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetClosestDistancePoint(const NormalizedCoords& coords) const
      -> const NormalizedCoords&;
  [[nodiscard]] static auto GetSpeedCoefficient(float baseSpeedCoeff,
                                                float sqDistFromZero,
                                                float amplitude,
                                                float sqDistMult,
                                                float sqDistOffset) -> float;
};

inline auto DistanceField::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                                [[maybe_unused]] const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> V2dFlt
{
  const NormalizedCoords& closestDistancePoint = GetClosestDistancePoint(coords);
  const float sqDistFromClosestPoint =
      UTILS::SqDistance(closestDistancePoint.GetX(), closestDistancePoint.GetY());

  if (m_params.mode0)
  {
    return {baseSpeedCoeffs.x + (m_params.xAmplitude * sqDistFromClosestPoint),
            baseSpeedCoeffs.y + (m_params.yAmplitude * sqDistFromClosestPoint)};
  }

  return {
      GetSpeedCoefficient(baseSpeedCoeffs.x, sqDistFromClosestPoint, m_params.xAmplitude,
                          m_params.xSqDistMult, m_params.xSqDistOffset),
      GetSpeedCoefficient(baseSpeedCoeffs.y, sqDistFromClosestPoint, m_params.yAmplitude,
                          m_params.ySqDistMult, m_params.ySqDistOffset),
  };
}

inline auto DistanceField::GetSpeedCoefficient(const float baseSpeedCoeff,
                                               const float sqDistFromZero,
                                               const float amplitude,
                                               const float sqDistMult,
                                               const float sqDistOffset) -> float
{
  return baseSpeedCoeff - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
}

inline auto DistanceField::GetParams() const -> const Params&
{
  return m_params;
}

inline void DistanceField::SetParams(const Params& params)
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
