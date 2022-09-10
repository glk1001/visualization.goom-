#include "tentacle_driver.h"

//#undef NO_LOGGING

#include "color/color_maps.h"
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
using UTILS::GRAPHICS::SmallImageBitmaps;
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
static constexpr auto START_MAIN_COLOR      = Pixel{5, 5, 5, MAX_ALPHA};
static constexpr auto ITER_ZERO_LERP_FACTOR = 0.9F;

static const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START =
    UTILS::MATH::SineWaveMultiplier{1.0F, -20.0F, +20.0F, 0.0F};

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps,
                               const CirclesTentacleLayout& tentacleLayout) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacleLayout{tentacleLayout},
    m_tentacleParams{NUM_TENTACLE_NODES, TENTACLE_LENGTH, 1.0F, ITER_ZERO_Y_VAL_WAVE_ZERO_START},
    m_tentaclePlotter{m_draw, m_goomRand, smallBitmaps},
    m_tentacles{GetTentacles()}
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

auto TentacleDriver::GetTentacles() const noexcept -> std::vector<Tentacle3D>
{
  // To hide the annoying flapping tentacle head, make near the start very dark.
  const auto startColors = MultiplePixels{START_MAIN_COLOR, START_MAIN_COLOR};

  auto tentacles = std::vector<Tentacle3D>{};

  tentacles.reserve(m_tentacleLayout.GetNumPoints());
  for (auto i = 0U; i < m_tentacleLayout.GetNumPoints(); ++i)
  {
    auto tentacle2D = CreateNewTentacle2D();

    auto tentacle =
        Tentacle3D{std::move(tentacle2D), startColors, Tentacle2D::MIN_NUM_NODES, m_goomRand};

    tentacle.SetStartPos(m_tentaclePlotter.GetCameraPosition() +
                         m_tentacleLayout.GetStartPoints().at(i));
    tentacle.SetEndPos(m_tentaclePlotter.GetCameraPosition() +
                       m_tentacleLayout.GetEndPoints().at(i));

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
  UpdateTentaclePlotter();
}

auto TentacleDriver::ChangeTentacleColorMaps() -> void
{
  std::for_each(
      begin(m_tentacles), end(m_tentacles), [](auto& tentacle) { tentacle.ChangeColorMaps(); });
}

inline auto TentacleDriver::UpdateTentaclePlotter() -> void
{
  m_tentaclePlotter.ChangeDotSizes();
  m_tentaclePlotter.ChangeNumNodesBetweenDots();
}

auto TentacleDriver::SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [this, &val](auto& tentacle)
                { tentacle.SetEndPosOffset(GetAdjustedCentrePos(val)); });
}

inline auto TentacleDriver::GetAdjustedCentrePos(const Point2dInt& val) const noexcept -> V3dFlt
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.5F;
  const auto acceptableCentrePos =
      lerp(Point2dInt{0, 0},
           GetPerspectiveAdjustedEndCentrePos(val) - Vec2dInt{m_screenMidpoint},
           CLOSE_TO_SCREEN_CENTRE_T);

  return {
      static_cast<float>(acceptableCentrePos.x), static_cast<float>(acceptableCentrePos.y), 0.0F};
}

auto TentacleDriver::GetPerspectiveAdjustedEndCentrePos(
    const Point2dInt& requestedCentrePos) const noexcept -> Point2dInt
{
  if (requestedCentrePos != m_screenMidpoint)
  {
    return requestedCentrePos;
  }

  const auto xOffset = m_goomRand.GetRandInRange(m_minPerspectiveAdjustmentOffset,
                                                 m_maxPerspectiveAdjustmentOffset + 1);
  const auto yOffset = m_goomRand.GetRandInRange(m_minPerspectiveAdjustmentOffset,
                                                 m_maxPerspectiveAdjustmentOffset + 1);

  const auto xOffsetSign = m_goomRand.ProbabilityOf(0.5F) ? -1 : +1;
  const auto yOffsetSign = m_goomRand.ProbabilityOf(0.5F) ? -1 : +1;

  return requestedCentrePos + Vec2dInt{xOffsetSign * xOffset, yOffsetSign * yOffset};
}

auto TentacleDriver::Update() -> void
{
  ++m_updateNum;

  CheckForTimerEvents();

  auto colorT = TValue{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(m_tentacles.size())};

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
        {m_dominantMainColorMap->GetColor(colorT()), m_dominantLowColorMap->GetColor(colorT())},
        m_dominantDotColorMap->GetColor(colorT()));

    m_tentaclePlotter.Plot3D(tentacle);

    colorT.Increment();
  }

  m_tentacleParams.iterZeroYValWave.Increment();
}

} // namespace GOOM::VISUAL_FX::TENTACLES
