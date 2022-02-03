#pragma once

#include "normalized_coords.h"
#include "point2d.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

class Speedway : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1,
    MODE2,
  };
  Speedway(Modes mode, const UTILS::IGoomRand& goomRand) noexcept;

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const Modes m_mode;
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  void SetMode2RandomParams();
  [[nodiscard]] auto GetMode0SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                               float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> Point2dFlt;
  [[nodiscard]] auto GetMode1SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                               float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> Point2dFlt;
  [[nodiscard]] auto GetMode2SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                               float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> Point2dFlt;
};

inline auto Speedway::GetSpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                           const float sqDistFromZero,
                                           const NormalizedCoords& coords) const -> Point2dFlt
{
  switch (m_mode)
  {
    case Modes::MODE0:
      return GetMode0SpeedCoefficients(baseSpeedCoeffs, sqDistFromZero, coords);
    case Modes::MODE1:
      return GetMode1SpeedCoefficients(baseSpeedCoeffs, sqDistFromZero, coords);
    case Modes::MODE2:
      return GetMode2SpeedCoefficients(baseSpeedCoeffs, sqDistFromZero, coords);
  }

  return {0.0F, 0.0F};
}

inline auto Speedway::GetMode0SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                                const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> Point2dFlt
{
  constexpr float SQ_DIST_FACTOR = 0.01F;
  float xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (constexpr float PROB_FLIP_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  const float xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + xAdd));
  const float ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetMode1SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                                const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> Point2dFlt
{
  float xAdd = -1.0F;

  constexpr float PROB_RANDOM_X_ADD = 0.5F;
  constexpr float PROB_FLIP_X_ADD = 0.5F;
  constexpr float PROB_NEGATIVE_X_ADD = 0.5F;

  if (m_goomRand.ProbabilityOf(PROB_RANDOM_X_ADD))
  {
    constexpr float MIN_NEGATIVE_X_ADD = -1.9F;
    constexpr float MAX_NEGATIVE_X_ADD = -0.5F;
    constexpr float MIN_POSITIVE_X_ADD = +0.5F;
    constexpr float MAX_POSITIVE_X_ADD = +1.9F;
    xAdd = m_goomRand.ProbabilityOf(PROB_NEGATIVE_X_ADD)
               ? m_goomRand.GetRandInRange(MIN_NEGATIVE_X_ADD, MAX_NEGATIVE_X_ADD)
               : m_goomRand.GetRandInRange(MIN_POSITIVE_X_ADD, MAX_POSITIVE_X_ADD);
  }
  else if (m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  const float xDiff = coords.GetX() - xAdd;
  const float sign = xDiff < 0.0F ? -1.0F : +1.0F;
  const float xWarp = 0.1F * (((sign * UTILS::Sq(xDiff)) / xAdd) + xAdd);
  const float amplitude = (1.0F - sqDistFromZero) / 4.0F;

  const float xSpeedCoeff = amplitude * baseSpeedCoeffs.x * (m_params.xAmplitude * xWarp);
  const float ySpeedCoeff = amplitude * m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetMode2SpeedCoefficients(const Point2dFlt& baseSpeedCoeffs,
                                                const float sqDistFromZero,
                                                const NormalizedCoords& coords) const -> Point2dFlt
{
  constexpr float SQ_DIST_FACTOR = 0.01F;
  float xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (constexpr float PROB_FLIP_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  const float xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + xAdd));
  const float ySpeedCoeff = std::tan(0.01F * sqDistFromZero) * m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetParams() const -> const Params&
{
  return m_params;
}

inline void Speedway::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
