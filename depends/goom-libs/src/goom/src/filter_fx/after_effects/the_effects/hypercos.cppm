module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Hypercos;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Hypercos
{
public:
  explicit Hypercos(const GoomRand& goomRand) noexcept;
  Hypercos(const Hypercos&) noexcept           = delete;
  Hypercos(Hypercos&&) noexcept                = delete;
  virtual ~Hypercos() noexcept                 = default;
  auto operator=(const Hypercos&) -> Hypercos& = delete;
  auto operator=(Hypercos&&) -> Hypercos&      = delete;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords,
                                 const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs;

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
  const GoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetMode0RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetMode1RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetMode2RandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetMode3RandomParams() const noexcept -> Params;
  Weights<HypercosEffect> m_hypercosOverlayWeights;
  [[nodiscard]] auto GetHypercosEffect(HypercosOverlayMode overlay,
                                       const NumberRange<float>& freqRange,
                                       const NumberRange<float>& amplitudeRange) const noexcept
      -> Params;
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

inline auto Hypercos::SetMode0RandomParams() -> void
{
  m_params = GetMode0RandomParams();
}

inline auto Hypercos::SetMode1RandomParams() -> void
{
  m_params = GetMode1RandomParams();
}

inline auto Hypercos::SetMode2RandomParams() -> void
{
  m_params = GetMode2RandomParams();
}

inline auto Hypercos::SetMode3RandomParams() -> void
{
  m_params = GetMode3RandomParams();
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
