#include "filter_zoom_vector_effects.h"

#include "filter_amulet.h"
#include "filter_crystal_ball.h"
#include "filter_data.h"
#include "filter_hypercos.h"
#include "filter_image_displacements.h"
#include "filter_normalized_coords.h"
#include "filter_planes.h"
#include "filter_scrunch.h"
#include "filter_simple_speed_coefficients_effect.h"
#include "filter_speedway.h"
#include "filter_wave.h"
#include "filter_y_only.h"
#include "goomutils/goomrand.h"
#include "goomutils/mathutils.h"
#include "v2d.h"

#include <cmath>
#include <cstdint>
#include <memory>
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

using UTILS::GetRandInRange;
using UTILS::m_half_pi;

ZoomVectorEffects::ZoomVectorEffects(const std::string& resourcesDirectory) noexcept
  : m_resourcesDirectory{resourcesDirectory},
    m_hypercos{std::make_unique<Hypercos>()},
    m_planes{std::make_unique<Planes>()}
{
}

ZoomVectorEffects::~ZoomVectorEffects() noexcept = default;

auto ZoomVectorEffects::MakeSpeedCoefficientsEffect(const ZoomFilterMode mode,
                                                    const std::string& resourcesDirectory)
    -> std::unique_ptr<SpeedCoefficientsEffect>
{
  switch (mode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_unique<Amulet>();
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE0);
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE1);
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_unique<ImageDisplacements>(resourcesDirectory);
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_unique<Scrunch>();
    case ZoomFilterMode::SPEEDWAY_MODE:
      return std::make_unique<Speedway>();
    case ZoomFilterMode::WAVE_MODE0:
      return std::make_unique<Wave>(Wave::Modes::MODE0);
    case ZoomFilterMode::WAVE_MODE1:
      return std::make_unique<Wave>(Wave::Modes::MODE1);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_unique<YOnly>();
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
    case ZoomFilterMode::NORMAL_MODE:
    case ZoomFilterMode::WATER_MODE:
      return std::make_unique<SimpleSpeedCoefficientsEffect>();
    default:
      throw std::logic_error("ZoomVectorEffects::SetFilterSettings: Unknown ZoomFilterMode.");
  }
}

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterData& filterSettings)
{
  m_filterSettings = &filterSettings;

  SetHypercosOverlaySettings();

  m_speedCoefficientsEffect =
      MakeSpeedCoefficientsEffect(filterSettings.mode, m_resourcesDirectory);

  m_speedCoefficientsEffect->SetRandomParams();
}

void ZoomVectorEffects::SetRandomPlaneEffects(const V2dInt& zoomMidPoint,
                                              const uint32_t screenWidth)
{
  m_planes->SetRandomParams(zoomMidPoint, screenWidth);
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

auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_hypercos->GetVelocity(coords);
}

auto ZoomVectorEffects::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_planes->IsHorizontalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetHorizontalPlaneVelocity(coords);
}

auto ZoomVectorEffects::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_planes->IsVerticalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetVerticalPlaneVelocity(coords);
}

auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  //    const float xAmp = 1.0/getRandInRange(50.0f, 200.0f);
  //    const float yAmp = 1.0/getRandInRange(50.0f, 200.0f);
  const float amp = (0.5F * m_filterSettings->noiseFactor) /
                    GetRandInRange(ZoomFilterData::NOISE_MIN, ZoomFilterData::NOISE_MAX);
  return {GetRandInRange(-amp, +amp), GetRandInRange(-amp, +amp)};
}

auto ZoomVectorEffects::GetTanEffectVelocity(const float sqDistFromZero,
                                             const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
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
    return {-m_filterSettings->rotateSpeed * (velocity.GetX() - velocity.GetY()),
            -m_filterSettings->rotateSpeed * (velocity.GetX() + velocity.GetY())};
  }

  return {m_filterSettings->rotateSpeed * (velocity.GetY() + velocity.GetX()),
          m_filterSettings->rotateSpeed * (velocity.GetY() - velocity.GetX())};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
