#ifndef VISUALIZATION_GOOM_FILTER_AMULET_H
#define VISUALIZATION_GOOM_FILTER_AMULET_H

#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Amulet
{
public:
  Amulet() noexcept;
  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs, float sqDistFromZero) const
      -> V2dFlt;

  void SetRandomParams();

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  void SetParams(const Params& params);

private:
  Params m_params;
};

inline auto Amulet::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                         const float sqDistFromZero) const -> V2dFlt
{
  return {baseSpeedCoeffs.x + (m_params.xAmplitude * sqDistFromZero),
          baseSpeedCoeffs.y + (m_params.yAmplitude * sqDistFromZero)};
  //?      speedCoeffs.y = 5.0F * std::cos(5.0F * speedCoeffs.x) * std::sin(5.0F * speedCoeffs.y);
}

inline auto Amulet::GetParams() const -> const Params&
{
  return m_params;
}

inline void Amulet::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_AMULET_H
