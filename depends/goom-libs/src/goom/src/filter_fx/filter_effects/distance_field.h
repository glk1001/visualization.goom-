#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class DistanceField : public IZoomInCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1,
    MODE2
  };
  explicit DistanceField(Modes mode, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero,
                                           const Point2dFlt& baseZoomInCoeffs) const noexcept
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

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
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  struct RelativeDistancePoint
  {
    float sqDistanceFromCoords;
    const NormalizedCoords& distancePoint;
  };
  auto SetMode0RandomParams() noexcept -> void;
  auto SetMode1RandomParams() noexcept -> void;
  auto SetMode2RandomParams() noexcept -> void;
  auto SetRandomParams(const UTILS::MATH::IGoomRand::NumberRange<float>& xAmplitudeRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& yAmplitudeRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistMultRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistMultRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistOffsetRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistOffsetRange,
                       std::vector<NormalizedCoords>&& distancePoints) noexcept -> void;
  [[nodiscard]] auto GetDistancePoints() const noexcept -> std::vector<NormalizedCoords>;
  [[nodiscard]] auto GetClosestDistancePoint(const NormalizedCoords& coords) const noexcept
      -> RelativeDistancePoint;
  [[nodiscard]] static auto GetZoomInCoefficient(float baseZoomInCoeff,
                                                 float sqDistFromZero,
                                                 float amplitude,
                                                 float sqDistMult,
                                                 float sqDistOffset) noexcept -> float;
};

inline auto DistanceField::GetZoomInCoefficients(const NormalizedCoords& coords,
                                                 [[maybe_unused]] float sqDistFromZero,
                                                 const Point2dFlt& baseZoomInCoeffs) const noexcept
    -> Point2dFlt
{
  const auto sqDistFromClosestPoint = GetClosestDistancePoint(coords).sqDistanceFromCoords;

  if (m_mode == Modes::MODE0)
  {
    return {baseZoomInCoeffs.x + (m_params.xAmplitude * sqDistFromClosestPoint),
            baseZoomInCoeffs.y + (m_params.yAmplitude * sqDistFromClosestPoint)};
  }

  return {
      GetZoomInCoefficient(baseZoomInCoeffs.x,
                           sqDistFromClosestPoint,
                           m_params.xAmplitude,
                           m_params.xSqDistMult,
                           m_params.xSqDistOffset),
      GetZoomInCoefficient(baseZoomInCoeffs.y,
                           sqDistFromClosestPoint,
                           m_params.yAmplitude,
                           m_params.ySqDistMult,
                           m_params.ySqDistOffset),
  };
}

inline auto DistanceField::GetZoomInCoefficient(const float baseZoomInCoeff,
                                                const float sqDistFromZero,
                                                const float amplitude,
                                                const float sqDistMult,
                                                const float sqDistOffset) noexcept -> float
{
  return baseZoomInCoeff - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
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
