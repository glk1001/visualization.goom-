module;

#include <cstdint>
#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Planes;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Planes
{
public:
  explicit Planes(const GoomRand& goomRand) noexcept;
  Planes(const Planes&) noexcept           = delete;
  Planes(Planes&&) noexcept                = delete;
  virtual ~Planes() noexcept               = default;
  auto operator=(const Planes&) -> Planes& = delete;
  auto operator=(Planes&&) -> Planes&      = delete;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const CoordsAndVelocity& coordsAndVelocity) const
      -> float;
  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const CoordsAndVelocity& coordsAndVelocity) const
      -> float;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs;

  virtual auto SetRandomParams(const Point2dInt& zoomMidpoint, uint32_t screenWidth) noexcept
      -> void;

  enum class PlaneSwirlType : UnderlyingEnumType
  {
    NONE,
    SIN_CURL_SWIRL,
    COS_CURL_SWIRL,
    SIN_COS_CURL_SWIRL,
    COS_SIN_CURL_SWIRL,
    //    SIN_TAN_CURL_SWIRL,
    //    COS_TAN_CURL_SWIRL,
    SIN_OF_COS_SWIRL,
    COS_OF_SIN_SWIRL,
  };
  struct PlaneEffects
  {
    bool horizontalEffectActive;
    bool verticalEffectActive;
    Amplitude amplitude;
  };
  struct PlaneSwirlEffects
  {
    PlaneSwirlType swirlType;
    FrequencyFactor frequencyFactor;
    Amplitude amplitude;
  };
  struct Params
  {
    PlaneEffects planeEffects;
    PlaneSwirlEffects swirlEffects;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const GoomRand* m_goomRand;
  enum class PlaneEffectEvents : UnderlyingEnumType
  {
    ZERO_EFFECTS,
    SMALL_EFFECTS,
    MEDIUM_EFFECTS,
    LARGE_EFFECTS,
    VERY_LARGE_EFFECTS,
    POS_HORIZONTAL_NEG_VERTICAL_VERY_LARGE_EFFECTS,
    POS_VERTICAL_NEG_HORIZONTAL_VERY_LARGE_EFFECTS,
  };
  UTILS::MATH::Weights<PlaneEffectEvents> m_planeEffectWeights;
  Params m_params;
  [[nodiscard]] auto GetRandomParams(const Point2dInt& zoomMidpoint,
                                     uint32_t screenWidth) const noexcept -> Params;

  [[nodiscard]] static auto GetRandomParams(const GoomRand& goomRand,
                                            PlaneEffectEvents planeEffectsEvent,
                                            const Point2dInt& zoomMidpoint,
                                            uint32_t screenWidth) -> Params;

  [[nodiscard]] static auto GetRandomPlaneEffects(const IntAmplitude& adjustedIntAmplitude,
                                                  const Amplitude& effectMultiplier)
      -> PlaneEffects;
  [[nodiscard]] static auto GetRandomPlaneEffects(const GoomRand& goomRand,
                                                  PlaneEffectEvents planeEffectsEvent,
                                                  bool muchSpiralling,
                                                  const Point2dInt& zoomMidpoint,
                                                  uint32_t screenWidth) -> PlaneEffects;
  [[nodiscard]] static auto GetRandomIntAmplitude(const GoomRand& goomRand,
                                                  PlaneEffectEvents planeEffectsEvent)
      -> IntAmplitude;
  [[nodiscard]] static auto GetAdjustedIntAmplitude(const GoomRand& goomRand,
                                                    const IntAmplitude& intAmplitude,
                                                    const Point2dInt& zoomMidpoint,
                                                    uint32_t screenWidth) -> IntAmplitude;
  [[nodiscard]] static auto GetRandomEffectMultiplier(const GoomRand& goomRand, bool muchSpiralling)
      -> Amplitude;

  [[nodiscard]] static auto GetZeroSwirlEffects() -> PlaneSwirlEffects;
  [[nodiscard]] static auto GetRandomSwirlEffects(const GoomRand& goomRand, bool muchSpiralling)
      -> PlaneSwirlEffects;
  [[nodiscard]] static auto GetNonzeroRandomSwirlEffects(const GoomRand& goomRand)
      -> PlaneSwirlEffects;
  [[nodiscard]] auto GetHorizontalSwirlOffsetFactor(float coordValue) const -> float;
  [[nodiscard]] auto GetVerticalSwirlOffsetFactor(float coordValue) const -> float;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto Planes::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_params.planeEffects.horizontalEffectActive;
}

inline auto Planes::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_params.planeEffects.verticalEffectActive;
}

inline auto Planes::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Planes::SetParams(const Params& params) -> void
{
  m_params = params;
}

inline auto Planes::SetRandomParams(const Point2dInt& zoomMidpoint,
                                    const uint32_t screenWidth) noexcept -> void
{
  m_params = GetRandomParams(zoomMidpoint, screenWidth);
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
