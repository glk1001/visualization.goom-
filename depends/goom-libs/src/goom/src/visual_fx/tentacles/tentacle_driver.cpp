#include "tentacle_driver.h"

//#undef NO_LOGGING

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "logging.h"
#include "tentacle3d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <functional>
#include <memory>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::TValue;
using UTILS::MATH::IGoomRand;

constexpr auto GetMatchingBaseYWeights(const float freq) noexcept -> Tentacle2D::BaseYWeights
{
  constexpr auto FREQUENCIES = std::array{
      1.1F,
      2.1F,
      3.1F,
      4.1F,
  };
  constexpr auto CORRESPONDING_BASE_Y_WEIGHTS = std::array{
      0.60F,
      0.70F,
      0.75F,
      0.80F,
  };
  constexpr auto HIGHEST_BASE_Y_WEIGHT = 0.85F;

  for (auto i = 0U; i < FREQUENCIES.size(); ++i)
  {
    if (freq <= FREQUENCIES.at(i))
    {
      return {CORRESPONDING_BASE_Y_WEIGHTS.at(i), 1.0F - CORRESPONDING_BASE_Y_WEIGHTS.at(i)};
    }
  }

  return {HIGHEST_BASE_Y_WEIGHT, 1.0F - HIGHEST_BASE_Y_WEIGHT};
}

static constexpr auto TENT2D_X_MIN          = 0.0;
static constexpr auto TENT2D_Y_MIN          = 0.065736;
static constexpr auto TENT2D_Y_MAX          = 10000.0;
static constexpr auto TENTACLE_LENGTH       = 120.0F;
static constexpr auto NUM_TENTACLE_NODES    = 100U;
static constexpr auto ITER_ZERO_LERP_FACTOR = 0.9F;

static const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START =
    UTILS::MATH::SineWaveMultiplier{1.0F, -20.0F, +20.0F, 0.0F};

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const IGoomRand& goomRand,
                               const CirclesTentacleLayout& tentacleLayout) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacleParams{NUM_TENTACLE_NODES, TENTACLE_LENGTH, 1.0F, ITER_ZERO_Y_VAL_WAVE_ZERO_START},
    m_tentaclePlotter{m_draw, m_goomRand},
    m_tentacles{GetTentacles(tentacleLayout)}
{
}

auto TentacleDriver::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&weightedColorMaps](auto& tentacle)
                { tentacle.SetWeightedColorMaps(weightedColorMaps); });
}

auto TentacleDriver::GetTentacles(const CirclesTentacleLayout& tentacleLayout) const noexcept
    -> std::vector<Tentacle3D>
{
  auto tentacles = std::vector<Tentacle3D>{};

  tentacles.reserve(tentacleLayout.GetNumPoints());
  for (auto i = 0U; i < tentacleLayout.GetNumPoints(); ++i)
  {
    auto tentacle2D = CreateNewTentacle2D();

    auto tentacle = Tentacle3D{std::move(tentacle2D), m_goomRand};

    tentacle.SetStartPos(tentacleLayout.GetStartPoints().at(i));
    tentacle.SetEndPos(tentacleLayout.GetEndPoints().at(i));

    tentacles.emplace_back(std::move(tentacle));
  }

  return tentacles;
}

auto TentacleDriver::CreateNewTentacle2D() const noexcept -> std::unique_ptr<Tentacle2D>
{
  const auto tentacleLen = m_tentacleParams.length;
  Ensures(tentacleLen >= 1.0F);
  const auto tent2dXMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  Ensures(tent2dXMax >= 1.0);

  const auto dimensions = Tentacle2D::Dimensions{
      {TENT2D_X_MIN,   tent2dXMax},
      {TENT2D_Y_MIN, TENT2D_Y_MAX},
  };
  const auto baseYWeights = GetMatchingBaseYWeights(m_tentacleParams.iterZeroYValWaveFreq);

  return std::make_unique<Tentacle2D>(m_tentacleParams.numNodes, dimensions, baseYWeights);
}

auto TentacleDriver::StartIterating() -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [](auto& tentacle) { tentacle.Get2DTentacle().StartIterating(); });

  m_updateNum = 0;
}

auto TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value) -> void
{
  const auto newFreq = value * m_tentacleParams.iterZeroYValWaveFreq;
  m_tentacleParams.iterZeroYValWave.SetFrequency(newFreq);

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    m_tentacles.at(i).Get2DTentacle().SetBaseYWeights(GetMatchingBaseYWeights(newFreq));
  }
}

auto TentacleDriver::CheckForTimerEvents() -> void
{
  if ((m_updateNum % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0U)
  {
    return;
  }

  ChangeTentacleColorMaps();

  m_tentaclePlotter.UpdateCameraPosition();
}

auto TentacleDriver::ChangeTentacleColorMaps() -> void
{
  std::for_each(
      begin(m_tentacles), end(m_tentacles), [](auto& tentacle) { tentacle.ChangeColorMaps(); });

  m_tentacleGroupSize =
      m_goomRand.GetRandInRange(MIN_TENTACLE_GROUP_SIZE, static_cast<uint32_t>(m_tentacles.size()));
}

auto TentacleDriver::SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void
{
  const auto acceptableCentreEndPosOffset = GetAcceptableCentrePosOffset(val);
  const auto radiusScale = m_goomRand.GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR);

  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&acceptableCentreEndPosOffset, &radiusScale](auto& tentacle)
                {
                  const auto newRadiusCentreEndPosOffset = GetNewRadiusCentrePosOffset(
                      radiusScale, tentacle.GetEndPos(), acceptableCentreEndPosOffset);

                  tentacle.SetEndPosOffset(newRadiusCentreEndPosOffset);
                });
}

inline auto TentacleDriver::GetAcceptableCentrePosOffset(
    const Point2dInt& requestedCentrePos) const noexcept -> Point2dInt
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.5F;
  return lerp(
      Point2dInt{0, 0}, requestedCentrePos - Vec2dInt{m_screenMidpoint}, CLOSE_TO_SCREEN_CENTRE_T);
}

auto TentacleDriver::GetNewRadiusCentrePosOffset(const float radiusScale,
                                                 const V3dFlt& endPosFlt,
                                                 const Point2dInt& oldCentreEndPosOffset) noexcept
    -> V3dFlt
{
  const auto endPosVec =
      Vec2dInt{static_cast<int32_t>(endPosFlt.x), static_cast<int32_t>(endPosFlt.y)};
  const auto newEndPos = Point2dInt{
      static_cast<int32_t>(radiusScale * static_cast<float>(endPosVec.x)),
      static_cast<int32_t>(radiusScale * static_cast<float>(endPosVec.y)),
  };
  const auto newRadiusEndPosOffset = newEndPos - endPosVec;

  const auto newCentreEndPosOffset = oldCentreEndPosOffset + Vec2dInt{newRadiusEndPosOffset};

  return V3dFlt{static_cast<float>(newCentreEndPosOffset.x),
                static_cast<float>(newCentreEndPosOffset.y),
                0.0F};
}

auto TentacleDriver::Update() -> void
{
  ++m_updateNum;

  CheckForTimerEvents();

  auto colorT = TValue{TValue::StepType::SINGLE_CYCLE, m_tentacleGroupSize};

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    auto& tentacle   = m_tentacles.at(i);
    auto& tentacle2D = tentacle.Get2DTentacle();

    tentacle.Update();

    const auto iterZeroYVal = m_tentacleParams.iterZeroYValWave.GetNext();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    m_tentaclePlotter.SetDominantColors(
        {m_dominantMainColorMap->GetColor(colorT()), m_dominantLowColorMap->GetColor(colorT())});

    m_tentaclePlotter.Plot3D(tentacle);

    colorT.Increment();
  }

  m_tentacleParams.iterZeroYValWave.Increment();
}

} // namespace GOOM::VISUAL_FX::TENTACLES
