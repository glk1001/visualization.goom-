#pragma once

#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class Planes
{
public:
  explicit Planes(const UTILS::IGoomRand& goomRand) noexcept;
  Planes(const Planes&) noexcept = delete;
  Planes(Planes&&) noexcept = delete;
  virtual ~Planes() noexcept = default;
  auto operator=(const Planes&) -> Planes& = delete;
  auto operator=(Planes&&) -> Planes& = delete;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  virtual void SetRandomParams(const V2dInt& zoomMidPoint, uint32_t screenWidth);

  enum class PlaneSwirlType
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
    _NUM // unused and must be last
  };
  struct Frequencies
  {
    float x;
    float y;
  };
  struct Amplitudes
  {
    float x;
    float y;
  };
  struct PlaneEffects
  {
    bool horizontalEffectActive;
    bool verticalEffectActive;
    Amplitudes amplitudes;
  };
  struct PlaneSwirlEffects
  {
    PlaneSwirlType swirlType;
    Frequencies frequencies;
    Amplitudes amplitudes;
  };
  struct Params
  {
    PlaneEffects planeEffects;
    PlaneSwirlEffects swirlEffects;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  enum class PlaneEffectEvents
  {
    ZERO_EFFECTS,
    SMALL_EFFECTS,
    MEDIUM_EFFECTS,
    LARGE_EFFECTS,
    VERY_LARGE_EFFECTS,
    POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS,
    POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS,
    _NUM // unused and must be last
  };
  const UTILS::Weights<PlaneEffectEvents> m_planeEffectWeights;

  [[nodiscard]] static auto GetRandomParams(const UTILS::IGoomRand& goomRand,
                                            PlaneEffectEvents planeEffectsEvent,
                                            const V2dInt& zoomMidPoint,
                                            uint32_t screenWidth) -> Params;

  [[nodiscard]] static auto GetRandomPlaneEffects(const UTILS::IGoomRand& goomRand,
                                                  PlaneEffectEvents planeEffectsEvent,
                                                  bool muchSpiralling,
                                                  const V2dInt& zoomMidPoint,
                                                  uint32_t screenWidth) -> PlaneEffects;
  struct IntAmplitudes
  {
    int32_t x;
    int32_t y;
  };
  [[nodiscard]] static auto GetRandomIntAmplitudes(const UTILS::IGoomRand& goomRand,
                                                   PlaneEffectEvents planeEffectsEvent)
      -> IntAmplitudes;
  [[nodiscard]] static auto GetAdjustedIntAmplitudes(const UTILS::IGoomRand& goomRand,
                                                     const IntAmplitudes& intAmplitudes,
                                                     const V2dInt& zoomMidPoint,
                                                     uint32_t screenWidth) -> IntAmplitudes;
  [[nodiscard]] static auto GetRandomEffectMultipliers(const UTILS::IGoomRand& goomRand,
                                                       bool muchSpiralling) -> Amplitudes;

  [[nodiscard]] static auto GetRandomSwirlEffects(const UTILS::IGoomRand& goomRand,
                                                  bool muchSpiralling) -> PlaneSwirlEffects;
  [[nodiscard]] auto GetHorizontalSwirlOffsetFactor(float coordValue) const -> float;
  [[nodiscard]] auto GetVerticalSwirlOffsetFactor(float coordValue) const -> float;
};

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

inline void Planes::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
