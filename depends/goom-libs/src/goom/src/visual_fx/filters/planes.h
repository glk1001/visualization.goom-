#pragma once

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

class Planes
{
public:
  explicit Planes(UTILS::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  void SetRandomParams(const V2dInt& zoomMidPoint, uint32_t screenWidth);

  struct Params
  {
    int horizontalEffect;
    float horizontalEffectAmplitude;
    int verticalEffect;
    float verticalEffectAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  UTILS::IGoomRand& m_goomRand;
  Params m_params;
  enum class PlaneEffectEvents
  {
    EVENT1,
    EVENT2,
    EVENT3,
    EVENT4,
    EVENT5,
    EVENT6,
    EVENT7,
    EVENT8
  };
  const UTILS::Weights<PlaneEffectEvents> m_planeEffectWeights;
};

inline auto Planes::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_params.horizontalEffect != 0;
}

inline auto Planes::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  // TODO - try xNormalized
  return coords.GetY() * m_params.horizontalEffectAmplitude *
         static_cast<float>(m_params.horizontalEffect);
}

inline auto Planes::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_params.verticalEffect != 0;
}

inline auto Planes::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  // TODO - try yNormalized
  return coords.GetX() * m_params.verticalEffectAmplitude *
         static_cast<float>(m_params.verticalEffect);
}

inline auto Planes::GetParams() const -> const Params&
{
  return m_params;
}

inline void Planes::SetParams(const Params& params)
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
