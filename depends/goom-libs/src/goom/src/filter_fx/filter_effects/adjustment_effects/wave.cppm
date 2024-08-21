module;

#include <cmath>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Wave;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Wave : public IZoomAdjustmentEffect
{
public:
  enum class Modes : UnderlyingEnumType
  {
    SQ_DIST_ANGLE_EFFECT_MODE0,
    SQ_DIST_ANGLE_EFFECT_MODE1,
    ATAN_ANGLE_EFFECT_MODE0,
    ATAN_ANGLE_EFFECT_MODE1,
  };
  Wave(Modes mode, const GoomRand& goomRand);

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  enum class AngleEffect : UnderlyingEnumType
  {
    ATAN,
    SQ_DIST,
    SQ_DIST_AND_SPIRAL
  };
  enum class WaveEffect : UnderlyingEnumType
  {
    WAVE_SIN_EFFECT,
    WAVE_COS_EFFECT,
    WAVE_SIN_COS_EFFECT,
    WAVE_TAN_EFFECT,
    WAVE_TAN_SIN_EFFECT,
    WAVE_TAN_COS_EFFECT,
    WAVE_COT_EFFECT,
    WAVE_COT_SIN_EFFECT,
    WAVE_COT_COS_EFFECT,
  };
  struct Params
  {
    Viewport viewport;
    WaveEffect xWaveEffect;
    WaveEffect yWaveEffect;
    AngleEffect angleEffect;
    float sqDistPower;
    float freqFactor;
    float amplitude;
    float periodicFactor;
    float reducerCoeff;
    float spiralRotateBaseAngle;
    bool useModifiedATanAngle;
    float modifiedATanAngleFactor;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  Modes m_mode;
  const GoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Weights<WaveEffect> m_weightedEffects;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetSqDistAngleEffectMode0RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetSqDistAngleEffectMode1RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetAtanAngleEffectMode0RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetAtanAngleEffectMode1RandomParams() const noexcept -> Params;

  struct WaveModeSettings
  {
    AngleEffect angleEffect{};
    NumberRange<float> freqFactorRange{};
    NumberRange<float> amplitudeRange{};
    NumberRange<float> periodicFactorRange{};
    NumberRange<float> sinCosPeriodicFactorRange{};
  };
  [[nodiscard]] auto GetWaveModeSettings(const WaveModeSettings& waveModeSettings) const noexcept
      -> Params;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetZoomAdjustmentAdd(WaveEffect waveEffect,
                                          float angle,
                                          float reducer) const noexcept -> float;
  [[nodiscard]] auto GetAngle(float sqDistFromZero,
                              const NormalizedCoords& coords) const noexcept -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) noexcept -> float;
  [[nodiscard]] auto GetPeriodicFactor(
      WaveEffect xWaveEffect,
      WaveEffect yWaveEffect,
      const NumberRange<float>& periodicFactorRange,
      const NumberRange<float>& sinCosPeriodicFactorRange) const noexcept -> float;
  [[nodiscard]] auto GetReducerCoeff(WaveEffect xWaveEffect,
                                     WaveEffect yWaveEffect,
                                     float periodicFactor) const noexcept -> float;

  [[nodiscard]] auto GetSqDistEffect() const noexcept -> Wave::AngleEffect;
  [[nodiscard]] auto GetATanAngle(const NormalizedCoords& coords) const noexcept -> float;
  [[nodiscard]] auto GetSqDistSpiralRotateAngle(
      float sqDistFromZero, const NormalizedCoords& coords) const noexcept -> float;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Wave::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {.x = coords.GetX() * velocity.x, .y = coords.GetY() * velocity.y};
}

inline auto Wave::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto Wave::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

inline auto Wave::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

inline auto Wave::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);

  const auto sqDistFromZero = SqDistanceFromZero(viewportCoords);
  const auto angle          = GetAngle(sqDistFromZero, viewportCoords);
  const auto reducer        = std::exp(-m_params.reducerCoeff * sqDistFromZero);

  const auto xZoomAdjustment =
      GetBaseZoomAdjustment().x + GetZoomAdjustmentAdd(m_params.xWaveEffect, angle, reducer);
  const auto yZoomAdjustment =
      GetBaseZoomAdjustment().y + GetZoomAdjustmentAdd(m_params.yWaveEffect, angle, reducer);

  return {.x = xZoomAdjustment, .y = yZoomAdjustment};
}

inline auto Wave::GetZoomAdjustmentAdd(const WaveEffect waveEffect,
                                       const float angle,
                                       const float reducer) const noexcept -> float
{
  return reducer * m_params.amplitude *
         GetPeriodicPart(waveEffect, m_params.freqFactor * angle, m_params.periodicFactor);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
