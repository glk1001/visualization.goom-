#pragma once

#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class CrystalBall : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1
  };
  explicit CrystalBall(Modes mode, UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float xSqDistMult;
    float xSqDistOffset;
    float yAmplitude;
    float ySqDistMult;
    float ySqDistOffset;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const Modes m_mode;
  UTILS::IGoomRand& m_goomRand;
  Params m_params;
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  void SetRandomParams(const UTILS::IGoomRand::NumberRange<float>& xAmplitudeRange,
                       const UTILS::IGoomRand::NumberRange<float>& yAmplitudeRange,
                       const UTILS::IGoomRand::NumberRange<float>& xSqDistMultRange,
                       const UTILS::IGoomRand::NumberRange<float>& ySqDistMultRange,
                       const UTILS::IGoomRand::NumberRange<float>& xSqDistOffsetRange,
                       const UTILS::IGoomRand::NumberRange<float>& ySqDistOffsetRange);
  [[nodiscard]] static auto GetSpeedCoefficient(float baseSpeedCoeff,
                                                float sqDistFromZero,
                                                float amplitude,
                                                float sqDistMult,
                                                float sqDistOffset) -> float;
};

inline auto CrystalBall::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                              const float sqDistFromZero,
                                              [[maybe_unused]] const NormalizedCoords& coords) const
    -> V2dFlt
{
  return {GetSpeedCoefficient(baseSpeedCoeffs.x, sqDistFromZero, m_params.xAmplitude,
                              m_params.xSqDistMult, m_params.xSqDistOffset),
          GetSpeedCoefficient(baseSpeedCoeffs.y, sqDistFromZero, m_params.yAmplitude,
                              m_params.ySqDistMult, m_params.ySqDistOffset)};
}

inline auto CrystalBall::GetSpeedCoefficient(const float baseSpeedCoeff,
                                             const float sqDistFromZero,
                                             const float amplitude,
                                             const float sqDistMult,
                                             const float sqDistOffset) -> float
{
  return baseSpeedCoeff - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
}

inline auto CrystalBall::GetParams() const -> const Params&
{
  return m_params;
}

inline void CrystalBall::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

