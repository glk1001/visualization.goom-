#ifndef VISUALIZATION_GOOM_FILTER_CRYSTAL_BALL_H
#define VISUALIZATION_GOOM_FILTER_CRYSTAL_BALL_H

#include "filter_normalized_coords.h"
#include "filter_speed_coefficients_effect.h"
#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"
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
  explicit CrystalBall(Modes mode) noexcept;

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
  Params m_params;
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  void SetRandomParams(const UTILS::NumberRange<float>& xAmplitudeRange,
                       const UTILS::NumberRange<float>& yAmplitudeRange,
                       const UTILS::NumberRange<float>& xSqDistMultRange,
                       const UTILS::NumberRange<float>& ySqDistMultRange,
                       const UTILS::NumberRange<float>& xSqDistOffsetRange,
                       const UTILS::NumberRange<float>& ySqDistOffsetRange);
  [[nodiscard]] static auto GetSpeedCoefficient(float baseCoordVal,
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

inline auto CrystalBall::GetSpeedCoefficient(const float baseCoordVal,
                                             const float sqDistFromZero,
                                             const float amplitude,
                                             const float sqDistMult,
                                             const float sqDistOffset) -> float
{
  return baseCoordVal - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
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

#endif //VISUALIZATION_GOOM_FILTER_CRYSTAL_BALL_H
