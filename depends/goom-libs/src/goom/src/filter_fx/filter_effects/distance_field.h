#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class DistanceField : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1,
    MODE2
  };
  explicit DistanceField(Modes mode, const GOOM::UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const NormalizedCoords& coords,
                                          float sqDistFromZero,
                                          const Point2dFlt& baseSpeedCoeffs) const noexcept
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
    float xSqDistMult;
    float ySqDistMult;
    float xSqDistOffset;
    float ySqDistOffset;
    std::vector<NormalizedCoords> distancePoints;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const Modes m_mode;
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  struct RelativeDistancePoint
  {
    float sqDistanceFromCoords;
    const NormalizedCoords& distancePoint;
  };
  auto SetMode0RandomParams() noexcept -> void;
  auto SetMode1RandomParams() noexcept -> void;
  auto SetMode2RandomParams() noexcept -> void;
  auto SetRandomParams(const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& xAmplitudeRange,
                       const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& yAmplitudeRange,
                       const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistMultRange,
                       const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistMultRange,
                       const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistOffsetRange,
                       const GOOM::UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistOffsetRange,
                       std::vector<NormalizedCoords>&& distancePoints) noexcept -> void;
  [[nodiscard]] auto GetDistancePoints() const noexcept -> std::vector<NormalizedCoords>;
  [[nodiscard]] auto GetClosestDistancePoint(const NormalizedCoords& coords) const noexcept
      -> RelativeDistancePoint;
  [[nodiscard]] static auto GetSpeedCoefficient(float baseSpeedCoeff,
                                                float sqDistFromZero,
                                                float amplitude,
                                                float sqDistMult,
                                                float sqDistOffset) noexcept -> float;
};

inline auto DistanceField::GetSpeedCoefficients(const NormalizedCoords& coords,
                                                [[maybe_unused]] const float sqDistFromZero,
                                                const Point2dFlt& baseSpeedCoeffs) const noexcept
    -> Point2dFlt
{
  const auto sqDistFromClosestPoint = GetClosestDistancePoint(coords).sqDistanceFromCoords;

  if (m_mode == Modes::MODE0)
  {
    return {baseSpeedCoeffs.x + (m_params.xAmplitude * sqDistFromClosestPoint),
            baseSpeedCoeffs.y + (m_params.yAmplitude * sqDistFromClosestPoint)};
  }

  return {
      GetSpeedCoefficient(baseSpeedCoeffs.x,
                          sqDistFromClosestPoint,
                          m_params.xAmplitude,
                          m_params.xSqDistMult,
                          m_params.xSqDistOffset),
      GetSpeedCoefficient(baseSpeedCoeffs.y,
                          sqDistFromClosestPoint,
                          m_params.yAmplitude,
                          m_params.ySqDistMult,
                          m_params.ySqDistOffset),
  };
}

inline auto DistanceField::GetSpeedCoefficient(const float baseSpeedCoeff,
                                               const float sqDistFromZero,
                                               const float amplitude,
                                               const float sqDistMult,
                                               const float sqDistOffset) noexcept -> float
{
  return baseSpeedCoeff - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
}

inline auto DistanceField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto DistanceField::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
