#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H

#include "filter_data.h"
#include "filter_normalized_coords.h"

#if __cplusplus <= 201402L
namespace GOOM
{

class FilterStats;

namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class ZoomVectorEffects
{
public:
  ZoomVectorEffects() noexcept;

  void SetFilterSettings(const ZoomFilterData& filterSettings);
  void SetFilterStats(FilterStats& stats);

  [[nodiscard]] auto GetMaxSpeedCoeff() const -> float;
  void SetMaxSpeedCoeff(float val);

  [[nodiscard]] auto GetStandardVelocity(float sqDistFromZero, const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] static auto GetCleanedVelocity(const NormalizedCoords& velocity)
      -> NormalizedCoords;
  [[nodiscard]] auto GetRotatedVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;
  [[nodiscard]] auto GetNoiseVelocity() const -> NormalizedCoords;
  [[nodiscard]] auto GetTanEffectVelocity(float sqDistFromZero,
                                          const NormalizedCoords& velocity) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetHPlaneEffectVelocity(const NormalizedCoords& coords) const -> float;
  [[nodiscard]] auto GetHypercosVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;
  [[nodiscard]] auto GetVPlaneEffectVelocity(const NormalizedCoords& coords) const -> float;

private:
  const ZoomFilterData* m_filterSettings{};
  mutable FilterStats* m_stats{};
  float m_maxSpeedCoeff = ZoomFilterData::DEFAULT_MAX_SPEED_COEFF;

  [[nodiscard]] static auto GetMinVelocityVal(float velocityVal) -> float;

  [[nodiscard]] auto GetSpeedCoeffVelocity(float sqDistFromZero,
                                           const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetImageDisplacementVelocity(const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetXYSpeedCoefficients(float sqDistFromZero,
                                            const NormalizedCoords& coords) const -> V2dFlt;
  [[nodiscard]] auto GetBaseSpeedCoefficients() const -> V2dFlt;
  [[nodiscard]] auto GetDefaultSpeedCoefficients() const -> V2dFlt;
  [[nodiscard]] auto GetAmuletSpeedCoefficients(float sqDistFromZero) const -> V2dFlt;
  [[nodiscard]] auto GetCrystalBallSpeedCoefficients(float sqDistFromZero) const -> V2dFlt;
  [[nodiscard]] auto GetScrunchSpeedCoefficients(float sqDistFromZero) const -> V2dFlt;
  [[nodiscard]] auto GetSpeedwaySpeedCoefficients(float sqDistFromZero,
                                                  const NormalizedCoords& coords) const -> V2dFlt;
  [[nodiscard]] auto GetWaveSpeedCoefficients(float sqDistFromZero) const -> V2dFlt;
  [[nodiscard]] auto GetYOnlySpeedCoefficients(const NormalizedCoords& coords,
                                               float sqDistFromZero) const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeffs(const V2dFlt& speedCoeffs) const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeff(const float speedCoeff) const -> float;
  [[nodiscard]] auto GetWaveEffectSpeedAdd(float sqDistFromZero,
                                           ZoomFilterData::WaveEffect waveEffect) const -> float;
  [[nodiscard]] auto GetWaveEffectPeriodicPart(float sqDistFromZero,
                                               ZoomFilterData::WaveEffect waveEffect) const
      -> float;
  [[nodiscard]] auto GetYOnlySpeedMultiplier(ZoomFilterData::YOnlyEffect yOnlyEffect,
                                             const NormalizedCoords& coords) const -> float;

  void UpdateDoZoomVectorSpeedCoeffBelowMinStats() const;
  void UpdateDoZoomVectorSpeedCoeffAboveMaxStats() const;
  void UpdateDoZoomVectorNoiseFactorStats() const;
  void UpdateDoZoomVectorTanEffectStats() const;
  void UpdateDoZoomVectorNegativeRotateStats() const;
  void UpdateDoZoomVectorPositiveRotateStats() const;
};

inline auto ZoomVectorEffects::GetMaxSpeedCoeff() const -> float
{
  return m_maxSpeedCoeff;
}

inline void ZoomVectorEffects::SetMaxSpeedCoeff(const float val)
{
  m_maxSpeedCoeff = val;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H
