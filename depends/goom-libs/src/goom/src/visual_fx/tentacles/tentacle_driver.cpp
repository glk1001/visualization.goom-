#include "tentacle_driver.h"

#include "color/color_maps.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "tentacle3d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

#include <cmath>
#include <functional>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
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
  constexpr auto HIGHEST_BASE_Y_WEIGHT = 0.90F;

  for (auto i = 0U; i < FREQUENCIES.size(); ++i)
  {
    if (freq <= FREQUENCIES.at(i))
    {
      return {CORRESPONDING_BASE_Y_WEIGHTS.at(i), 1.0F - CORRESPONDING_BASE_Y_WEIGHTS.at(i)};
    }
  }

  return {HIGHEST_BASE_Y_WEIGHT, 1.0F - HIGHEST_BASE_Y_WEIGHT};
}

static constexpr auto ITER_ZERO_LERP_FACTOR = 0.9F;

static constexpr auto TENT2D_X_MIN = 0.0;
static constexpr auto TENT2D_Y_MIN = 0.065736;
static constexpr auto TENT2D_Y_MAX = 10000.0;

static constexpr auto START_MAIN_COLOR = Pixel{5, 5, 5, MAX_ALPHA};

static constexpr auto MIN_NUM_NODES_FACTOR                 = 1.0F;
static constexpr auto MAX_NUM_NODES_FACTOR                 = 1.0001F;
static constexpr auto MIN_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.0F;
static constexpr auto MAX_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.0001F;
static constexpr auto MIN_TENTACLE_LENGTH                  = 100.0F;
static constexpr auto MAX_LENGTH_FACTOR                    = 1.0001F;

static const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START =
    UTILS::MATH::SineWaveMultiplier{1.0F, -20.0F, +20.0F, 0.0F};
static const auto ITER_ZERO_Y_VAL_WAVE_PI_START =
    UTILS::MATH::SineWaveMultiplier{1.0F, -25.0F, +25.0F, 0.0F};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP1_FIRST{
    100, 100.0F, 1.0F, ITER_ZERO_Y_VAL_WAVE_ZERO_START};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP1_LAST{
    100, 120.0F, 1.1F, ITER_ZERO_Y_VAL_WAVE_ZERO_START};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP2_FIRST{
    100, 100.0F, 1.1F, ITER_ZERO_Y_VAL_WAVE_PI_START};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP2_LAST{
    100, 120.0F, 1.5F, ITER_ZERO_Y_VAL_WAVE_PI_START};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP3_FIRST{
    100, 100.0F, 1.5F, ITER_ZERO_Y_VAL_WAVE_PI_START};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP3_LAST{
    100, 120.0F, 2.0F, ITER_ZERO_Y_VAL_WAVE_PI_START};

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps,
                               const ITentacleLayout& tentacleLayout) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacleLayout{tentacleLayout},
    m_iterParamsGroups{
        { m_goomRand, ITER_PARAM_GROUP1_FIRST, ITER_PARAM_GROUP1_LAST },
//        { m_goomRand, ITER_PARAM_GROUP2_FIRST, ITER_PARAM_GROUP2_LAST },
//        { m_goomRand, ITER_PARAM_GROUP3_FIRST, ITER_PARAM_GROUP3_LAST },
    },
    m_tentaclePlotter{m_draw, m_goomRand, smallBitmaps}
{
  // TODO(glk) - pretty clunky.
  UpdateTentaclePositions(m_tentacles, tentacleLayout);
}

auto TentacleDriver::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&weightedColorMaps](auto& tentacle)
                { tentacle.SetWeightedColorMaps(weightedColorMaps); });
}

auto TentacleDriver::GetTentacleParams(const size_t numTentacles,
                                       const std::vector<IterParamsGroup>& iterParamsGroups)
    -> std::vector<IterationParams>
{
  auto tentacleParams        = std::vector<IterationParams>{};
  const auto numInParamGroup = numTentacles / iterParamsGroups.size();

  // IMPORTANT: Dividing the numTentacles into param groups makes the tentacle movements
  //            look good. I tried making them all move the same, but it just didn't look
  //            right. For the moment this seems best.
  const auto tStep = 1.0F / static_cast<float>(numInParamGroup - 1);
  auto paramsIndex = 0U;
  auto t           = 0.0F;
  tentacleParams.reserve(numTentacles);
  for (auto i = 0U; i < numTentacles; ++i)
  {
    const auto& paramsGroup = iterParamsGroups.at(paramsIndex);
    if (0 == (i % numInParamGroup))
    {
      if (paramsIndex < (iterParamsGroups.size() - 1))
      {
        ++paramsIndex;
      }
      t = 0.0;
    }
    tentacleParams.emplace_back(paramsGroup.GetNextIterationParams(t));
    t += tStep;
  }

  return tentacleParams;
}

auto TentacleDriver::GetTentacles(const IGoomRand& goomRand,
                                  const std::vector<IterationParams>& tentacleParams)
    -> std::vector<Tentacle3D>
{
  auto tentacles = std::vector<Tentacle3D>{};

  tentacles.reserve(tentacleParams.size());
  for (auto i = 0U; i < tentacleParams.size(); ++i)
  {
    auto tentacle2D = CreateNewTentacle2D(goomRand, tentacleParams[i]);

    // To hide the annoying flapping tentacle head, make near the start very dark.
    const auto startColors = MultiplePixels{START_MAIN_COLOR, START_MAIN_COLOR};

    auto tentacle =
        Tentacle3D{std::move(tentacle2D), startColors, Tentacle2D::MIN_NUM_NODES, goomRand};

    tentacles.emplace_back(std::move(tentacle));
  }

  return tentacles;
}

auto TentacleDriver::IterParamsGroup::GetNextIterationParams(const float t) const
    -> TentacleDriver::IterationParams
{
  auto params = IterationParams{
      /* .numNodes = */
      static_cast<uint32_t>(
          goomRand.GetRandInRange(MIN_NUM_NODES_FACTOR, MAX_NUM_NODES_FACTOR) *
          STD20::lerp(static_cast<float>(first.numNodes), static_cast<float>(last.numNodes), t)),
      /* .length = */
      goomRand.GetRandInRange(MIN_TENTACLE_LENGTH,
                              MAX_LENGTH_FACTOR * STD20::lerp(first.length, last.length, t)),
      /* .iterZeroYValWaveFreq = */
      goomRand.GetRandInRange(MIN_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR,
                              MAX_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR) *
          STD20::lerp(first.iterZeroYValWaveFreq, last.iterZeroYValWaveFreq, t),
      /* .iterZeroYValWave = */
      first.iterZeroYValWave,
  };

  Ensures(params.numNodes >= 10);
  Ensures(params.length >= MIN_TENTACLE_LENGTH);

  return params;
}

auto TentacleDriver::CreateNewTentacle2D(const IGoomRand& goomRand, const IterationParams& params)
    -> std::unique_ptr<Tentacle2D>
{
  const auto tentacleLen = std::max(1.0F, goomRand.GetRandInRange(0.99F, 1.01F) * params.length);
  Ensures(tentacleLen >= 1.0F);
  const auto tent2dXMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  Ensures(tent2dXMax >= 1.0);

  const auto dimensions = Tentacle2D::Dimensions{
      {TENT2D_X_MIN,   tent2dXMax},
      {TENT2D_Y_MIN, TENT2D_Y_MAX},
  };
  const auto baseYWeights = GetMatchingBaseYWeights(params.iterZeroYValWaveFreq);

  return std::make_unique<Tentacle2D>(params.numNodes, dimensions, baseYWeights);
}

auto TentacleDriver::StartIterating() -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [](auto& tentacle) { tentacle.Get2DTentacle().StartIterating(); });

  m_updateNum = 0;
}

auto TentacleDriver::UpdateTentaclePositions(std::vector<Tentacle3D>& tentacles,
                                             const ITentacleLayout& tentacleLayout) const noexcept
    -> void
{
  // We want the longest tentacles to have start positions with the largest x values.
  for (auto i = 0U; i < tentacles.size(); ++i)
  {
    const auto startPos = GetStartPos(tentacleLayout.GetStartPoints().at(i));
    tentacles[m_sortedIndexesLongestFirst.at(i)].SetStartPos(startPos);
    tentacles[m_sortedIndexesLongestFirst.at(i)].SetEndPos(tentacleLayout.GetEndPoints().at(i));
  }

  // To help with perspective, any tentacles near vertical centre will be shortened.
  std::for_each(begin(tentacles),
                end(tentacles),
                [](auto& tentacle) { ShortenTentacleIfNearVerticalCentre(tentacle); });
}

auto TentacleDriver::ShortenTentacleIfNearVerticalCentre(Tentacle3D& tentacle) -> void
{
  if (const auto& startPos = tentacle.GetStartPos();
      std::fabs(startPos.x) > Tentacle3D::START_SMALL_X)
  {
    return;
  }

  auto& tentacle2D   = tentacle.Get2DTentacle();
  const auto xMin    = tentacle2D.GetXMin();
  const auto xMax    = tentacle2D.GetXMax();
  const auto newXMax = xMin + (0.9 * (xMax - xMin));
  tentacle2D.SetXDimensions({xMin, newXMax});
}

auto TentacleDriver::GetStartPos(const V3dFlt& tentacleLayoutStartPos) -> V3dFlt
{
  auto startPos = tentacleLayoutStartPos;

  if ((0.0F <= startPos.x) && (startPos.x < Tentacle3D::START_SMALL_X))
  {
    startPos.x = Tentacle3D::START_SMALL_X;
  }
  else if ((-Tentacle3D::START_SMALL_X < startPos.x) && (startPos.x <= 0.0F))
  {
    startPos.x = -Tentacle3D::START_SMALL_X;
  }

  return startPos;
}

auto TentacleDriver::GetSortedIndexesLongestTentaclesFirst(const std::vector<Tentacle3D>& tentacles)
    -> std::vector<size_t>
{
  const auto compareByLength = [&tentacles](const size_t id1, const size_t id2)
  {
    const auto len1 = tentacles[id1].Get2DTentacle().GetTentacleLength();
    const auto len2 = tentacles[id2].Get2DTentacle().GetTentacleLength();
    return len1 > len2; // Sort by longest first.
  };

  auto sortedLongestFirst = std::vector<size_t>(tentacles.size());
  std::iota(begin(sortedLongestFirst), end(sortedLongestFirst), 0);
  std::sort(begin(sortedLongestFirst), end(sortedLongestFirst), compareByLength);

  return sortedLongestFirst;
}

auto TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value) -> void
{
  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    const auto newFreq = value * m_tentacleParams.at(i).iterZeroYValWaveFreq;
    m_tentacleParams.at(i).iterZeroYValWave.SetFrequency(newFreq);
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

inline auto TentacleDriver::UpdateTentaclePlotter() -> void
{
  m_tentaclePlotter.ChangeDotSizes();
  m_tentaclePlotter.ChangeNumNodesBetweenDots();
}

auto TentacleDriver::ChangeTentacleColorMaps() -> void
{
  std::for_each(
      begin(m_tentacles), end(m_tentacles), [](auto& tentacle) { tentacle.ChangeColorMaps(); });
}

auto TentacleDriver::SetAllTentaclesStartCentrePos(const Point2dInt& val) noexcept -> void
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.2F;
  auto acceptableCentrePos =
      lerp(Point2dInt{0, 0}, val - Vec2dInt{m_screenMidpoint}, CLOSE_TO_SCREEN_CENTRE_T);
  acceptableCentrePos       = {0, 0};
  const V3dFlt allCentrePos = {
      static_cast<float>(acceptableCentrePos.x), static_cast<float>(acceptableCentrePos.y), 0.0F};

  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&allCentrePos](auto& tentacle) { tentacle.SetStartPosOffset(allCentrePos); });
}

auto TentacleDriver::SetAllTentaclesEndCentrePos(const Point2dInt& val) noexcept -> void
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.2F;
  const auto acceptableCentrePos =
      lerp(Point2dInt{0, 0}, val - Vec2dInt{m_screenMidpoint}, CLOSE_TO_SCREEN_CENTRE_T);
  const V3dFlt allCentrePos = {
      static_cast<float>(acceptableCentrePos.x), static_cast<float>(acceptableCentrePos.y), 0.0F};

  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&allCentrePos](auto& tentacle) { tentacle.SetEndPosOffset(allCentrePos); });

  // Make a small offset for start centre to help wih vertical perspective weirdness.
  static constexpr auto CLOSE_TO_ZERO_CENTRE_T = 0.05F;
  const auto acceptableStartCentreX = STD20::lerp(0.0F, -allCentrePos.x, CLOSE_TO_ZERO_CENTRE_T);
  const V3dFlt allStartCentrePos    = {acceptableStartCentreX, 0.0F, 0.0F};

  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&allStartCentrePos](auto& tentacle)
                { tentacle.SetStartPosOffset(allStartCentrePos); });
}

auto TentacleDriver::Update() -> void
{
  ++m_updateNum;

  CheckForTimerEvents();

  auto colorT = TValue{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(m_tentacles.size())};

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    auto& tentacle   = m_tentacles[m_sortedIndexesLongestFirst.at(i)];
    auto& tentacle2D = tentacle.Get2DTentacle();

    tentacle.Update();

    const auto iterZeroYVal = m_tentacleParams[i].iterZeroYValWave.GetNext();
    m_tentacleParams[i].iterZeroYValWave.Increment();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    m_tentaclePlotter.SetDominantColors(
        {m_dominantMainColorMap->GetColor(colorT()), m_dominantLowColorMap->GetColor(colorT())},
        m_dominantDotColorMap->GetColor(colorT()));

    m_tentaclePlotter.Plot3D(tentacle);

    colorT.Increment();
  }
}

} // namespace GOOM::VISUAL_FX::TENTACLES
