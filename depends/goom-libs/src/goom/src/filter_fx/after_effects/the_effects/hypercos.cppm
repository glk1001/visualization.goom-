module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Hypercos;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Hypercos
{
public:
  explicit Hypercos(const UTILS::MATH::IGoomRand& goomRand) noexcept;
  Hypercos(const Hypercos&) noexcept           = delete;
  Hypercos(Hypercos&&) noexcept                = delete;
  virtual ~Hypercos() noexcept                 = default;
  auto operator=(const Hypercos&) -> Hypercos& = delete;
  auto operator=(Hypercos&&) -> Hypercos&      = delete;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords,
                                 const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  virtual auto SetDefaultParams() -> void;
  virtual auto SetMode0RandomParams() -> void;
  virtual auto SetMode1RandomParams() -> void;
  virtual auto SetMode2RandomParams() -> void;
  virtual auto SetMode3RandomParams() -> void;

  enum class HypercosEffect : UnderlyingEnumType
  {
    NONE,
    SIN_CURL_SWIRL,
    COS_CURL_SWIRL,
    SIN_COS_CURL_SWIRL,
    COS_SIN_CURL_SWIRL,
    SIN_TAN_CURL_SWIRL,
    COS_TAN_CURL_SWIRL,
    SIN_RECTANGULAR,
    COS_RECTANGULAR,
    SIN_OF_COS_SWIRL,
    COS_OF_SIN_SWIRL,
  };

  struct Params
  {
    HypercosOverlayMode overlay;
    HypercosEffect effect;
    bool reverse;
    FrequencyFactor frequencyFactor;
    Amplitude amplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
  UTILS::MATH::Weights<HypercosEffect> m_hypercosOverlayWeights;
  auto SetHypercosEffect(HypercosOverlayMode overlay,
                         const UTILS::MATH::NumberRange<float>& freqRange,
                         const UTILS::MATH::NumberRange<float>& amplitudeRange) -> void;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords,
                                 HypercosEffect effect,
                                 const FrequencyFactor& frequencyFactorToUse) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetFrequencyFactorToUse(float frequencyFactor) const -> float;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto Hypercos::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Hypercos::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
