#include "tentacle_driver.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "tentacle3d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/mathutils.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <functional>
#include <memory>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::GetHalf;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::m_pi;

const size_t TentacleDriver::CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400;

// clang-format off
static const UTILS::MATH::SineWaveMultiplier ITER_ZERO_Y_VAL_WAVE_ZERO_START{
    1.0F, -10.0F, +10.0F, 0.0F
};
static const UTILS::MATH::SineWaveMultiplier ITER_ZERO_Y_VAL_WAVE_PI_START{
    1.5F, -10.0F, +10.0F, m_pi
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP1_FIRST{
    100, 0.600F, 100.0F, 1.0F, ITER_ZERO_Y_VAL_WAVE_PI_START
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP1_LAST{
    125, 0.700F, 105.0F, 2.0F, ITER_ZERO_Y_VAL_WAVE_ZERO_START
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP2_FIRST{
    125, 0.700F, 100.0F, 0.5F, ITER_ZERO_Y_VAL_WAVE_ZERO_START
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP2_LAST{
    150, 0.800F, 105.0F, 1.5F, ITER_ZERO_Y_VAL_WAVE_PI_START
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP3_FIRST{
    150, 0.800F, 100.0F, 1.5F, ITER_ZERO_Y_VAL_WAVE_PI_START
};
const TentacleDriver::IterationParams TentacleDriver::ITER_PARAM_GROUP3_LAST{
    200, 0.900F, 105.0F, 2.5F, ITER_ZERO_Y_VAL_WAVE_ZERO_START
};
// clang-format on

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const IGoomRand& goomRand,
                               const SmallImageBitmaps& smallBitmaps,
                               const ITentacleLayout& tentacleLayout) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_tentacleLayout{tentacleLayout},
    m_iterParamsGroups{
        { m_goomRand, ITER_PARAM_GROUP1_FIRST, ITER_PARAM_GROUP1_LAST },
        { m_goomRand, ITER_PARAM_GROUP2_FIRST, ITER_PARAM_GROUP2_LAST },
        { m_goomRand, ITER_PARAM_GROUP3_FIRST, ITER_PARAM_GROUP3_LAST },
    },
    m_tentacleParams{GetTentacleParams(m_tentacleLayout.GetNumPoints(), m_iterParamsGroups)},
    m_tentacles{GetTentacles(m_goomRand, m_tentacleParams, m_tentacleLayout)},
    m_tentaclePlotter{m_draw, m_goomRand, smallBitmaps}
{
}

void TentacleDriver::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps>& weightedMaps)
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.SetWeightedColorMaps(weightedMaps);
  }
}

constexpr double TENT2D_X_MIN = 0.0;
constexpr double TENT2D_Y_MIN = 0.065736;
constexpr double TENT2D_Y_MAX = 10000.0;

auto TentacleDriver::GetTentacleParams(const size_t numTentacles,
                                       const std::vector<IterParamsGroup>& iterParamsGroups)
    -> std::vector<IterationParams>
{
  std::vector<IterationParams> tentacleParams{};
  const size_t numInParamGroup = numTentacles / iterParamsGroups.size();

  // IMPORTANT: Dividing the numTentacles into param groups makes the tentacle movements
  //            look good. I tried making them all move the same, but it just didn't look
  //            right. For the moment this seems best.
  const float tStep = 1.0F / static_cast<float>(numInParamGroup - 1);
  size_t paramsIndex = 0;
  float t = 0.0F;
  tentacleParams.reserve(numTentacles);
  for (size_t i = 0; i < numTentacles; ++i)
  {
    const IterParamsGroup paramsGroup = iterParamsGroups.at(paramsIndex);
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
                                  const std::vector<IterationParams>& tentacleParams,
                                  const ITentacleLayout& tentacleLayout) -> std::vector<Tentacle3D>
{
  std::vector<Tentacle3D> tentacles{};

  tentacles.reserve(tentacleParams.size());
  for (size_t i = 0; i < tentacleParams.size(); ++i)
  {
    std::unique_ptr<Tentacle2D> tentacle2D{CreateNewTentacle2D(goomRand, i, tentacleParams[i])};

    // To hide the annoying flapping tentacle head, make near the head very dark.
    constexpr V3dFlt INITIAL_HEAD_POS = {0, 0, 0};
    const auto headColor = Pixel{5, 5, 5, MAX_ALPHA};
    const Pixel headLowColor = headColor;
    Tentacle3D tentacle{std::move(tentacle2D),     headColor, headLowColor, INITIAL_HEAD_POS,
                        Tentacle2D::MIN_NUM_NODES, goomRand};

    tentacles.emplace_back(std::move(tentacle));
  }

  UpdateTentaclesLayout(tentacles, tentacleLayout);

  return tentacles;
}

auto TentacleDriver::IterParamsGroup::GetNextIterationParams(const float t) const
    -> TentacleDriver::IterationParams
{
  constexpr float MIN_NUM_NODES_FACTOR = 1.0F;
  constexpr float MAX_NUM_NODES_FACTOR = 1.1F;
  constexpr float MIN_PREV_Y_WEIGHT_FACTOR = 1.0F;
  constexpr float MAX_PREV_Y_WEIGHT_FACTOR = 1.1F;
  constexpr float MIN_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.0F;
  constexpr float MAX_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.1F;
  constexpr float MIN_LENGTH = 1.0F;
  constexpr float MAX_LENGTH_FACTOR = 1.1F;

  IterationParams params{
      /* .numNodes = */
      static_cast<size_t>(
          goomRand.GetRandInRange(MIN_NUM_NODES_FACTOR, MAX_NUM_NODES_FACTOR) *
          STD20::lerp(static_cast<float>(first.numNodes), static_cast<float>(last.numNodes), t)),
      /* .prevYWeight = */
      goomRand.GetRandInRange(MIN_PREV_Y_WEIGHT_FACTOR, MAX_PREV_Y_WEIGHT_FACTOR) *
          STD20::lerp(first.prevYWeight, last.prevYWeight, t),
      /* .length = */
      goomRand.GetRandInRange(MIN_LENGTH,
                              MAX_LENGTH_FACTOR * STD20::lerp(first.length, last.length, t)),
      /* .iterZeroYValWaveFreq = */
      goomRand.GetRandInRange(MIN_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR,
                              MAX_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR) *
          STD20::lerp(first.iterZeroYValWaveFreq, last.iterZeroYValWaveFreq, t),
      /* .iterZeroYValWave = */
      first.iterZeroYValWave,
  };

  assert(params.numNodes >= 10);
  assert(params.length >= MIN_LENGTH);

  return params;
}

auto TentacleDriver::CreateNewTentacle2D(const IGoomRand& goomRand,
                                         const size_t id,
                                         const IterationParams& params)
    -> std::unique_ptr<Tentacle2D>
{
  const float tentacleLen = std::max(1.0F, goomRand.GetRandInRange(0.99F, 1.01F) * params.length);
  assert(tentacleLen >= 1.0F);
  const double tent2dXMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  assert(tent2dXMax >= 1.0);

  std::unique_ptr<Tentacle2D> tentacle{
      std::make_unique<Tentacle2D>(id, params.numNodes, TENT2D_X_MIN, tent2dXMax, TENT2D_Y_MIN,
                                   TENT2D_Y_MAX, params.prevYWeight, 1.0F - params.prevYWeight)};

  tentacle->SetDoDamping(true);

  return tentacle;
}

void TentacleDriver::StartIterating()
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.Get2DTentacle().StartIterating();
  }

  m_updateNum = 0;
}

void TentacleDriver::UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                           const ITentacleLayout& tentacleLayout)
{
  std::vector<size_t> sortedLongestFirst(tentacles.size());
  std::iota(sortedLongestFirst.begin(), sortedLongestFirst.end(), 0);
  const auto compareByLength = [&](const size_t id1, const size_t id2)
  {
    const double len1 = tentacles[id1].Get2DTentacle().GetLength();
    const double len2 = tentacles[id2].Get2DTentacle().GetLength();
    // Sort by longest first.
    return len1 > len2;
  };
  std::sort(sortedLongestFirst.begin(), sortedLongestFirst.end(), compareByLength);

  for (size_t i = 0; i < tentacles.size(); ++i)
  {
    V3dFlt head = tentacleLayout.GetPoints().at(i);

    if ((0.0F <= head.x) && (head.x < Tentacle3D::HEAD_SMALL_X))
    {
      head.x = Tentacle3D::HEAD_SMALL_X;
    }
    else if ((-Tentacle3D::HEAD_SMALL_X < head.x) && (head.x <= 0.0F))
    {
      head.x = -Tentacle3D::HEAD_SMALL_X;
    }

    tentacles[sortedLongestFirst.at(i)].SetHead(head);
  }

  // To help with perspective, any tentacles near vertical centre will be shortened.
  for (auto& tentacle : tentacles)
  {
    const V3dFlt& head = tentacle.GetHead();
    if (std::fabs(head.x) <= Tentacle3D::HEAD_SMALL_X)
    {
      Tentacle2D& tentacle2D = tentacle.Get2DTentacle();
      const double xMin = tentacle2D.GetXMin();
      const double xMax = tentacle2D.GetXMax();
      const double newXMax = xMin + (1.0 * (xMax - xMin));
      tentacle2D.SetXDimensions(xMin, newXMax);
      constexpr size_t NUM_NODE_FACTOR = 6;
      tentacle.SetNumHeadNodes(std::max(NUM_NODE_FACTOR * Tentacle2D::MIN_NUM_NODES,
                                        GetHalf(tentacle.Get2DTentacle().GetNumNodes())));
    }
  }
}

void TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value)
{
  for (auto& params : m_tentacleParams)
  {
    const float newFreq = value * params.iterZeroYValWaveFreq;
    params.iterZeroYValWave.SetFrequency(newFreq);
  }
}

void TentacleDriver::SetReverseColorMix(const bool value)
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.SetReverseColorMix(value);
  }
}

void TentacleDriver::CheckForTimerEvents()
{
  if ((m_updateNum % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0)
  {
    return;
  }

  TentaclesColorMapsChanged();

  m_tentaclePlotter.ChangeDotSizes();
  m_tentaclePlotter.ChangeNumNodesBetweenDots();
}

void TentacleDriver::TentaclesColorMapsChanged()
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.ColorMapsChanged();
  }
}

void TentacleDriver::Update()
{
  ++m_updateNum;

  CheckForTimerEvents();

  constexpr float ITER_ZERO_LERP_FACTOR = 0.9F;

  for (size_t i = 0; i < m_tentacles.size(); ++i)
  {
    Tentacle3D& tentacle = m_tentacles[i];
    Tentacle2D& tentacle2D = tentacle.Get2DTentacle();

    const float iterZeroYVal = m_tentacleParams[i].iterZeroYValWave.GetNext();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    m_tentaclePlotter.Plot3D(tentacle);
  }
}

} // namespace GOOM::VISUAL_FX::TENTACLES
