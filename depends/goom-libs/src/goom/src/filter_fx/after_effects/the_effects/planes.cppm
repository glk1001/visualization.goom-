module;

#include <cstdint>
#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Planes;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Planes
{
public:
  explicit Planes(const UTILS::MATH::IGoomRand& goomRand) noexcept;
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

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  virtual auto SetRandomParams(const Point2dInt& zoomMidpoint, uint32_t screenWidth) -> void;

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
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
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

  [[nodiscard]] static auto GetRandomParams(const UTILS::MATH::IGoomRand& goomRand,
                                            PlaneEffectEvents planeEffectsEvent,
                                            const Point2dInt& zoomMidpoint,
                                            uint32_t screenWidth) -> Params;

  [[nodiscard]] static auto GetRandomPlaneEffects(
      const IntAmplitude& adjustedIntAmplitude, const Amplitude& effectMultiplier) -> PlaneEffects;
  [[nodiscard]] static auto GetRandomPlaneEffects(const UTILS::MATH::IGoomRand& goomRand,
                                                  PlaneEffectEvents planeEffectsEvent,
                                                  bool muchSpiralling,
                                                  const Point2dInt& zoomMidpoint,
                                                  uint32_t screenWidth) -> PlaneEffects;
  [[nodiscard]] static auto GetRandomIntAmplitude(
      const UTILS::MATH::IGoomRand& goomRand, PlaneEffectEvents planeEffectsEvent) -> IntAmplitude;
  [[nodiscard]] static auto GetAdjustedIntAmplitude(const UTILS::MATH::IGoomRand& goomRand,
                                                    const IntAmplitude& intAmplitude,
                                                    const Point2dInt& zoomMidpoint,
                                                    uint32_t screenWidth) -> IntAmplitude;
  [[nodiscard]] static auto GetRandomEffectMultiplier(const UTILS::MATH::IGoomRand& goomRand,
                                                      bool muchSpiralling) -> Amplitude;

  [[nodiscard]] static auto GetZeroSwirlEffects() -> PlaneSwirlEffects;
  [[nodiscard]] static auto GetRandomSwirlEffects(const UTILS::MATH::IGoomRand& goomRand,
                                                  bool muchSpiralling) -> PlaneSwirlEffects;
  [[nodiscard]] static auto GetNonzeroRandomSwirlEffects(const UTILS::MATH::IGoomRand& goomRand)
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

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
