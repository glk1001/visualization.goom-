#include "tentacle_driver.h"

#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "tentacle3d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <cmath>
#include <functional>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using STD20::pi;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;

const size_t TentacleDriver::CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400;

// clang-format off
static const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START = UTILS::MATH::SineWaveMultiplier{
    1.0F, -15.0F, +15.0F, 0.0F
};
static const auto ITER_ZERO_Y_VAL_WAVE_PI_START = UTILS::MATH::SineWaveMultiplier{
    1.5F, -15.0F, +15.0F, pi
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
    m_tentaclePlotter{m_draw, m_goomRand, smallBitmaps}
{
}

auto TentacleDriver::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps>& weightedMaps) -> void
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.SetWeightedColorMaps(weightedMaps);
  }
}

static constexpr auto TENT2D_X_MIN = 0.0;
static constexpr auto TENT2D_Y_MIN = 0.065736;
static constexpr auto TENT2D_Y_MAX = 10000.0;

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
                                  const std::vector<IterationParams>& tentacleParams,
                                  const ITentacleLayout& tentacleLayout) -> std::vector<Tentacle3D>
{
  auto tentacles = std::vector<Tentacle3D>{};

  tentacles.reserve(tentacleParams.size());
  for (auto i = 0U; i < tentacleParams.size(); ++i)
  {
    auto tentacle2D = CreateNewTentacle2D(goomRand, i, tentacleParams[i]);

    // To hide the annoying flapping tentacle head, make near the head very dark.
    static constexpr auto INITIAL_HEAD_POS = V3dFlt{0, 0, 0};
    const auto headMainColor               = Pixel{5, 5, 5, MAX_ALPHA};
    const auto headLowColor                = headMainColor;

    auto tentacle = Tentacle3D{std::move(tentacle2D),
                               headMainColor,
                               headLowColor,
                               INITIAL_HEAD_POS,
                               Tentacle2D::MIN_NUM_NODES,
                               goomRand};

    tentacles.emplace_back(std::move(tentacle));
  }

  UpdateTentaclesLayout(tentacles, tentacleLayout);

  return tentacles;
}

auto TentacleDriver::IterParamsGroup::GetNextIterationParams(const float t) const
    -> TentacleDriver::IterationParams
{
  static constexpr auto MIN_NUM_NODES_FACTOR                 = 1.0F;
  static constexpr auto MAX_NUM_NODES_FACTOR                 = 1.1F;
  static constexpr auto MIN_PREV_Y_WEIGHT_FACTOR             = 1.0F;
  static constexpr auto MAX_PREV_Y_WEIGHT_FACTOR             = 1.1F;
  static constexpr auto MIN_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.0F;
  static constexpr auto MAX_ITER_ZERO_Y_VAL_WAVE_FREQ_FACTOR = 1.1F;
  static constexpr auto MIN_LENGTH                           = 1.0F;
  static constexpr auto MAX_LENGTH_FACTOR                    = 1.1F;

  auto params = IterationParams{
      /* .numNodes = */
      static_cast<uint32_t>(
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

  Ensures(params.numNodes >= 10);
  Ensures(params.length >= MIN_LENGTH);

  return params;
}

auto TentacleDriver::CreateNewTentacle2D(const IGoomRand& goomRand,
                                         const uint32_t id,
                                         const IterationParams& params)
    -> std::unique_ptr<Tentacle2D>
{
  const auto tentacleLen = std::max(1.0F, goomRand.GetRandInRange(0.99F, 1.01F) * params.length);
  assert(tentacleLen >= 1.0F);
  const auto tent2dXMax = TENT2D_X_MIN + static_cast<double>(tentacleLen);
  assert(tent2dXMax >= 1.0);

  auto tentacle = std::make_unique<Tentacle2D>(id,
                                               params.numNodes,
                                               TENT2D_X_MIN,
                                               tent2dXMax,
                                               TENT2D_Y_MIN,
                                               TENT2D_Y_MAX,
                                               params.prevYWeight,
                                               1.0F - params.prevYWeight);

  tentacle->SetDoDamping(true);

  return tentacle;
}

auto TentacleDriver::StartIterating() -> void
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.Get2DTentacle().StartIterating();
  }

  m_updateNum = 0;
}

auto TentacleDriver::UpdateTentaclesLayout(std::vector<Tentacle3D>& tentacles,
                                           const ITentacleLayout& tentacleLayout) -> void
{
  auto sortedLongestFirst = std::vector<size_t>(tentacles.size());
  std::iota(begin(sortedLongestFirst), end(sortedLongestFirst), 0);
  const auto compareByLength = [&](const size_t id1, const size_t id2)
  {
    const auto len1 = tentacles[id1].Get2DTentacle().GetLength();
    const auto len2 = tentacles[id2].Get2DTentacle().GetLength();
    // Sort by longest first.
    return len1 > len2;
  };
  std::sort(begin(sortedLongestFirst), end(sortedLongestFirst), compareByLength);

  for (auto i = 0U; i < tentacles.size(); ++i)
  {
    auto head = tentacleLayout.GetPoints().at(i);

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
    const auto& head = tentacle.GetHead();
    if (std::fabs(head.x) <= Tentacle3D::HEAD_SMALL_X)
    {
      auto& tentacle2D   = tentacle.Get2DTentacle();
      const auto xMin    = tentacle2D.GetXMin();
      const auto xMax    = tentacle2D.GetXMax();
      const auto newXMax = xMin + (1.0 * (xMax - xMin));
      tentacle2D.SetXDimensions(xMin, newXMax);
      static constexpr auto NUM_NODE_FACTOR = 6U;
      tentacle.SetNumHeadNodes(std::max(NUM_NODE_FACTOR * Tentacle2D::MIN_NUM_NODES,
                                        U_HALF * tentacle.Get2DTentacle().GetNumNodes()));
    }
  }
}

auto TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value) -> void
{
  for (auto& params : m_tentacleParams)
  {
    const auto newFreq = value * params.iterZeroYValWaveFreq;
    params.iterZeroYValWave.SetFrequency(newFreq);
  }
}

auto TentacleDriver::SetReverseColorMix(const bool value) -> void
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.SetReverseColorMix(value);
  }
}

auto TentacleDriver::CheckForTimerEvents() -> void
{
  if ((m_updateNum % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0)
  {
    return;
  }

  TentaclesColorMapsChanged();

  m_tentaclePlotter.ChangeDotSizes();
  m_tentaclePlotter.ChangeNumNodesBetweenDots();
}

auto TentacleDriver::TentaclesColorMapsChanged() -> void
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.ColorMapsChanged();
  }
}

auto TentacleDriver::Update() -> void
{
  ++m_updateNum;

  CheckForTimerEvents();

  static constexpr auto ITER_ZERO_LERP_FACTOR = 0.9F;

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    auto& tentacle   = m_tentacles[i];
    auto& tentacle2D = tentacle.Get2DTentacle();

    const auto iterZeroYVal = m_tentacleParams[i].iterZeroYValWave.GetNext();
    tentacle2D.SetIterZeroLerpFactor(static_cast<double>(ITER_ZERO_LERP_FACTOR));
    tentacle2D.SetIterZeroYVal(static_cast<double>(iterZeroYVal));

    tentacle2D.Iterate();

    m_tentaclePlotter.Plot3D(tentacle);
  }
}

} // namespace GOOM::VISUAL_FX::TENTACLES
