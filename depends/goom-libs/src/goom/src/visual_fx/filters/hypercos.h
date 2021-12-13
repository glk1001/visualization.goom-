#pragma once

#include "filter_settings.h"
#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class Hypercos
{
public:
  explicit Hypercos(const UTILS::IGoomRand& goomRand) noexcept;
  Hypercos(const Hypercos&) noexcept = delete;
  Hypercos(Hypercos&&) noexcept = delete;
  virtual ~Hypercos() noexcept = default;
  auto operator=(const Hypercos&) -> Hypercos& = delete;
  auto operator=(Hypercos&&) -> Hypercos& = delete;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  virtual void SetDefaultParams();
  virtual void SetMode0RandomParams();
  virtual void SetMode1RandomParams();
  virtual void SetMode2RandomParams();
  virtual void SetMode3RandomParams();

  enum class HypercosEffect
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
    _NUM // unused and must be last
  };

  struct Params
  {
    HypercosOverlay overlay;
    HypercosEffect effect;
    bool reverse;
    float xFreq;
    float yFreq;
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  const UTILS::IGoomRand& m_goomRand;
  Params m_params;
  const UTILS::Weights<HypercosEffect> m_hypercosOverlayWeights;
  void SetHypercosEffect(HypercosOverlay overlay,
                         const UTILS::IGoomRand::NumberRange<float>& freqRange,
                         const UTILS::IGoomRand::NumberRange<float>& amplitudeRange);
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords,
                                 HypercosEffect effect,
                                 float xFreqToUse,
                                 float yFreqToUse) const -> NormalizedCoords;
  [[nodiscard]] auto GetFreqToUse(float freq) const -> float;
};

inline auto Hypercos::GetParams() const -> const Params&
{
  return m_params;
}

inline void Hypercos::SetParams(const Params& params)
{
  m_params = params;
}

} // namespace GOOM::VISUAL_FX::FILTERS
