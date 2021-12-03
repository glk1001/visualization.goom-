#pragma once

#include "filter_settings.h"
#include "normalized_coords.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

class Hypercos
{
public:
  explicit Hypercos(const UTILS::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  void SetDefaultParams();
  void SetMode0RandomParams();
  void SetMode1RandomParams();
  void SetMode2RandomParams();
  void SetMode3RandomParams();

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
  void SetHypercosEffect(const UTILS::IGoomRand::NumberRange<float>& freqRange,
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
