#ifndef VISUALIZATION_GOOM_LIB_GOOM_VISUAL_FX_FILTERS_HYPERCOS_H
#define VISUALIZATION_GOOM_LIB_GOOM_VISUAL_FX_FILTERS_HYPERCOS_H

#include "filter_settings.h"
#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"
#include "normalized_coords.h"
#include "v2d.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Hypercos
{
public:
  Hypercos() noexcept;

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
  Params m_params;
  void SetHypercosEffect(const UTILS::NumberRange<float>& freqRange,
                         const UTILS::NumberRange<float>& amplitudeRange);
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
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_LIB_GOOM_VISUAL_FX_FILTERS_HYPERCOS_H
