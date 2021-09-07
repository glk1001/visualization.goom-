#ifndef VISUALIZATION_GOOM_FILTER_PLANES_H
#define VISUALIZATION_GOOM_FILTER_PLANES_H

#include "filter_normalized_coords.h"
#include "v2d.h"

#include <string>
#include <tuple>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Planes
{
public:
  Planes() noexcept;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto GetNameValueParams() const -> std::vector<std::pair<std::string, std::string>>;

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
  Params m_params;
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
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_PLANES_H
