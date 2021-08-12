#ifndef VISUALIZATION_GOOM_FILTER_SCRUNCH_H
#define VISUALIZATION_GOOM_FILTER_SCRUNCH_H

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

class Scrunch
{
public:
  Scrunch() noexcept;
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

inline auto Scrunch::GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                         const float sqDistFromZero) const -> V2dFlt
{
  const float xSpeedCoeff = baseSpeedCoeffs.x + m_params.xAmplitude * sqDistFromZero;
  const float ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;
  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Scrunch::GetParams() const -> const Params&
{
  return m_params;
}

inline void Scrunch::SetParams(const Params& params)
{
  m_params = params;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_SCRUNCH_H
