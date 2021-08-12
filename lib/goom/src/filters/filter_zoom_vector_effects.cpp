#include "filter_zoom_vector_effects.h"

#include "../stats/filter_stats.h"
#include "filter_amulet.h"
#include "filter_crystal_ball.h"
#include "filter_data.h"
#include "filter_hypercos.h"
#include "filter_normalized_coords.h"
#include "filter_scrunch.h"
#include "filter_speedway.h"
#include "filter_wave.h"
#include "filter_y_only.h"
#include "image_displacement.h"

#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetRandInRange;
using UTILS::m_half_pi;
using UTILS::SMALL_FLOAT;

ZoomVectorEffects::ZoomVectorEffects() noexcept
  : m_amulet{std::make_unique<Amulet>()},
    m_crystalBall{std::make_unique<CrystalBall>()},
    m_hypercos{std::make_unique<Hypercos>()},
    m_scrunch{std::make_unique<Scrunch>()},
    m_speedway{std::make_unique<Speedway>()},
    m_wave{std::make_unique<Wave>()},
    m_yOnly{std::make_unique<YOnly>()}
{
}

ZoomVectorEffects::~ZoomVectorEffects() noexcept = default;

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterData& filterSettings)
{
  m_filterSettings = &filterSettings;

  SetHypercosOverlaySettings();

  switch (m_filterSettings->mode)
  {
    case ZoomFilterMode::AMULET_MODE:
      m_amulet->SetRandomParams();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      m_crystalBall->SetMode0RandomParams();
      break;
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      m_crystalBall->SetMode1RandomParams();
      break;
    case ZoomFilterMode::SCRUNCH_MODE:
      m_scrunch->SetRandomParams();
      break;
    case ZoomFilterMode::SPEEDWAY_MODE:
      m_speedway->SetRandomParams();
      break;
    case ZoomFilterMode::WAVE_MODE0:
      m_wave->SetMode0RandomParams();
      break;
    case ZoomFilterMode::WAVE_MODE1:
      m_wave->SetMode1RandomParams();
      break;
    case ZoomFilterMode::Y_ONLY_MODE:
      m_yOnly->SetRandomParams();
      break;
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
      // Hypercos modes already handled with 'SetHypercosOverlaySettings()' above
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
    case ZoomFilterMode::NORMAL_MODE:
    case ZoomFilterMode::WATER_MODE:
      break;
    default:
      throw std::logic_error("ZoomVectorEffects::SetFilterSettings: Unknown ZoomFilterMode.");
  }
}

void ZoomVectorEffects::SetHypercosOverlaySettings()
{
  switch (m_filterSettings->hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_hypercos->SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_hypercos->SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_hypercos->SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_hypercos->SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_hypercos->SetMode3RandomParams();
      break;
  }
}

void ZoomVectorEffects::SetFilterStats(FilterStats& stats)
{
  m_stats = &stats;
}

void ZoomVectorEffects::UpdateLastStats()
{
  m_stats->SetLastAmuletParams(m_amulet->GetParams());
  m_stats->SetLastCrystalBallParams(m_crystalBall->GetParams());
  m_stats->SetLastHypercosParams(m_hypercos->GetParams());
  m_stats->SetLastScrunchParams(m_scrunch->GetParams());
  m_stats->SetLastSpeedwayParams(m_speedway->GetParams());
  m_stats->SetLastWaveParams(m_wave->GetParams());
  m_stats->SetLastYOnlyParams(m_yOnly->GetParams());
}

auto ZoomVectorEffects::GetCleanedVelocity(const NormalizedCoords& velocity) -> NormalizedCoords
{
  return {GetMinVelocityVal(velocity.GetX()), GetMinVelocityVal(velocity.GetY())};
}

inline auto ZoomVectorEffects::GetMinVelocityVal(const float velocityVal) -> float
{
  if (std::fabs(velocityVal) < NormalizedCoords::GetMinNormalizedCoordVal())
  {
    return velocityVal < 0.0F ? -NormalizedCoords::GetMinNormalizedCoordVal()
                              : NormalizedCoords::GetMinNormalizedCoordVal();
  }
  return velocityVal;
}

auto ZoomVectorEffects::GetStandardVelocity(const float sqDistFromZero,
                                            const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  NormalizedCoords velocity = m_filterSettings->mode == ZoomFilterMode::IMAGE_DISPLACEMENT_MODE
                                  ? GetImageDisplacementVelocity(coords)
                                  : GetSpeedCoeffVelocity(sqDistFromZero, coords);

  return velocity;
}

inline auto ZoomVectorEffects::GetImageDisplacementVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  if (m_filterSettings->imageDisplacement == nullptr)
  {
    throw std::logic_error("No image displacement map setup.");
  }

  return NormalizedCoords{
      m_filterSettings->imageDisplacement->GetDisplacementVector(coords.ToFlt())};
}

inline auto ZoomVectorEffects::GetSpeedCoeffVelocity(const float sqDistFromZero,
                                                     const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  const V2dFlt speedCoeffs = GetClampedSpeedCoeffs(GetXYSpeedCoefficients(sqDistFromZero, coords));
  return {speedCoeffs.x * coords.GetX(), speedCoeffs.y * coords.GetY()};
}

auto ZoomVectorEffects::GetXYSpeedCoefficients(float sqDistFromZero,
                                               const NormalizedCoords& coords) const -> V2dFlt
{
  switch (m_filterSettings->mode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return GetAmuletSpeedCoefficients(sqDistFromZero);

    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return GetCrystalBallSpeedCoefficients(sqDistFromZero);

    case ZoomFilterMode::SCRUNCH_MODE:
      return GetScrunchSpeedCoefficients(sqDistFromZero);

    case ZoomFilterMode::SPEEDWAY_MODE:
      return GetSpeedwaySpeedCoefficients(sqDistFromZero, coords);

    case ZoomFilterMode::WAVE_MODE0:
    case ZoomFilterMode::WAVE_MODE1:
      return GetWaveSpeedCoefficients(sqDistFromZero);

    case ZoomFilterMode::Y_ONLY_MODE:
      return GetYOnlySpeedCoefficients(sqDistFromZero, coords);

    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
    case ZoomFilterMode::NORMAL_MODE:
      return GetBaseSpeedCoefficients();

    // Amulet 2
    // vx = X * tan(dist);
    // vy = Y * tan(dist);
    default:
      throw std::logic_error(std20::format("Switch: unhandled case '{}'.", m_filterSettings->mode));
  }
}

inline auto ZoomVectorEffects::GetBaseSpeedCoefficients() const -> V2dFlt
{
  const float speedCoeff = (1.0F + m_filterSettings->vitesse.GetRelativeSpeed()) /
                           ZoomFilterData::SPEED_COEFF_DENOMINATOR;
  return {speedCoeff, speedCoeff};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeffs(const V2dFlt& speedCoeffs) const -> V2dFlt
{
  return {GetClampedSpeedCoeff(speedCoeffs.x), GetClampedSpeedCoeff(speedCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeff(const float speedCoeff) const -> float
{
  if (speedCoeff < ZoomFilterData::MIN_SPEED_COEFF)
  {
    UpdateDoZoomVectorSpeedCoeffBelowMinStats();
    return ZoomFilterData::MIN_SPEED_COEFF;
  }
  if (speedCoeff > m_maxSpeedCoeff)
  {
    UpdateDoZoomVectorSpeedCoeffAboveMaxStats();
    return m_maxSpeedCoeff;
  }
  return speedCoeff;
}

inline auto ZoomVectorEffects::GetAmuletSpeedCoefficients(const float sqDistFromZero) const
    -> V2dFlt
{
  return m_amulet->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero);
}

inline auto ZoomVectorEffects::GetCrystalBallSpeedCoefficients(const float sqDistFromZero) const
    -> V2dFlt
{
  return m_crystalBall->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero);
}

inline auto ZoomVectorEffects::GetScrunchSpeedCoefficients(const float sqDistFromZero) const
    -> V2dFlt
{
  return m_scrunch->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero);
}

inline auto ZoomVectorEffects::GetSpeedwaySpeedCoefficients(const float sqDistFromZero,
                                                            const NormalizedCoords& coords) const
    -> V2dFlt
{
  return m_speedway->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero, coords);
}

inline auto ZoomVectorEffects::GetWaveSpeedCoefficients(const float sqDistFromZero) const -> V2dFlt
{
  return m_wave->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero);
}

inline auto ZoomVectorEffects::GetYOnlySpeedCoefficients(const float sqDistFromZero,
                                                         const NormalizedCoords& coords) const
    -> V2dFlt
{
  return m_yOnly->GetSpeedCoefficients(GetBaseSpeedCoefficients(), sqDistFromZero, coords);
}

auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_hypercos->GetVelocity(coords);
}

auto ZoomVectorEffects::GetHPlaneEffectVelocity(const NormalizedCoords& coords) const -> float
{
  // TODO - try xNormalized
  return coords.GetY() * m_filterSettings->hPlaneEffectAmplitude *
         static_cast<float>(m_filterSettings->hPlaneEffect);
}

auto ZoomVectorEffects::GetVPlaneEffectVelocity(const NormalizedCoords& coords) const -> float
{
  // TODO - try yNormalized
  return coords.GetX() * m_filterSettings->vPlaneEffectAmplitude *
         static_cast<float>(m_filterSettings->vPlaneEffect);
}

auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  if (m_filterSettings->noiseFactor < SMALL_FLOAT)
  {
    return {0.0, 0.0};
  }

  UpdateDoZoomVectorNoiseFactorStats();
  //    const float xAmp = 1.0/getRandInRange(50.0f, 200.0f);
  //    const float yAmp = 1.0/getRandInRange(50.0f, 200.0f);
  const float amp = 0.5F * m_filterSettings->noiseFactor /
                    GetRandInRange(ZoomFilterData::NOISE_MIN, ZoomFilterData::NOISE_MAX);
  return {GetRandInRange(-amp, +amp), GetRandInRange(-amp, +amp)};
}

auto ZoomVectorEffects::GetTanEffectVelocity(const float sqDistFromZero,
                                             const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  UpdateDoZoomVectorTanEffectStats();
  const float tanArg =
      stdnew::clamp(std::fmod(sqDistFromZero, m_half_pi), -0.85F * m_half_pi, 0.85F * m_half_pi);
  const float tanSqDist = std::tan(tanArg);
  return {tanSqDist * velocity.GetX(), tanSqDist * velocity.GetY()};
}

auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  if (m_filterSettings->rotateSpeed < 0.0F)
  {
    UpdateDoZoomVectorNegativeRotateStats();
    return {-m_filterSettings->rotateSpeed * (velocity.GetX() - velocity.GetY()),
            -m_filterSettings->rotateSpeed * (velocity.GetX() + velocity.GetY())};
  }

  UpdateDoZoomVectorPositiveRotateStats();
  return {m_filterSettings->rotateSpeed * (velocity.GetY() + velocity.GetX()),
          m_filterSettings->rotateSpeed * (velocity.GetY() - velocity.GetX())};
}

inline void ZoomVectorEffects::UpdateDoZoomVectorSpeedCoeffBelowMinStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorSpeedCoeffBelowMin();
}

inline void ZoomVectorEffects::UpdateDoZoomVectorSpeedCoeffAboveMaxStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorSpeedCoeffAboveMax();
}

inline void ZoomVectorEffects::UpdateDoZoomVectorNoiseFactorStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorNoiseFactor();
}

inline void ZoomVectorEffects::UpdateDoZoomVectorTanEffectStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorTanEffect();
}

inline void ZoomVectorEffects::UpdateDoZoomVectorNegativeRotateStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorNegativeRotate();
}

inline void ZoomVectorEffects::UpdateDoZoomVectorPositiveRotateStats() const
{
  if (m_stats == nullptr)
  {
    return;
  }
  m_stats->DoZoomVectorPositiveRotate();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
